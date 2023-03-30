/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "update_agent.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common/uuid/uuid.h"
#include "img_dir_serializer.h"
#include "protocols/service/fwu/packed-c/fwu_proto.h"
#include "protocols/service/fwu/packed-c/status.h"
#include "service/fwu/fw_store/fw_store.h"
#include "service/fwu/inspector/fw_inspector.h"

static bool open_image_directory(struct update_agent *update_agent, const struct uuid_octets *uuid,
				 uint32_t *handle, int *status);

static bool open_fw_store_object(struct update_agent *update_agent, const struct uuid_octets *uuid,
				 uint32_t *handle, int *status);

static bool open_fw_image(struct update_agent *update_agent, const struct uuid_octets *uuid,
			  uint32_t *handle, int *status);

int update_agent_init(struct update_agent *update_agent, unsigned int boot_index,
		      fw_inspector_inspect fw_inspect_method, struct fw_store *fw_store)
{
	assert(update_agent);
	assert(fw_inspect_method);
	assert(fw_store);

	int status = FWU_STATUS_UNKNOWN;

	update_agent->state = FWU_STATE_INITIALIZING;
	update_agent->fw_inspect_method = fw_inspect_method;
	update_agent->fw_store = fw_store;
	update_agent->image_dir_buf_size = 0;
	update_agent->image_dir_buf = NULL;

	stream_manager_init(&update_agent->stream_manager);

	/* Initialize and populate the fw_directory. The fw_inspector will
	 * obtain trustworthy information about the booted firmware and
	 * populate the fw_directory to reflect information about the booted
	 * firmware.
	 */
	fw_directory_init(&update_agent->fw_directory);

	status = update_agent->fw_inspect_method(&update_agent->fw_directory, boot_index);
	if (status != FWU_STATUS_SUCCESS)
		return status;

	/* Allow the associated fw_store to synchronize its state to the
	 * state of the booted firmware reflected by the fw_directory.
	 */
	status = fw_store_synchronize(update_agent->fw_store, &update_agent->fw_directory,
				      boot_index);
	if (status != FWU_STATUS_SUCCESS)
		return status;

	/* Allocate a buffer for holding the serialized image directory  */
	update_agent->image_dir_buf_size = img_dir_serializer_get_len(&update_agent->fw_directory);
	update_agent->image_dir_buf = malloc(update_agent->image_dir_buf_size);
	if (!update_agent->image_dir_buf)
		return FWU_STATUS_UNKNOWN;

	/* Transition to initial state */
	update_agent->state = fw_store_is_trial(update_agent->fw_store) ? FWU_STATE_TRIAL :
									  FWU_STATE_REGULAR;

	return FWU_STATUS_SUCCESS;
}

void update_agent_deinit(struct update_agent *update_agent)
{
	update_agent->state = FWU_STATE_DEINITIALZED;

	free(update_agent->image_dir_buf);
	fw_directory_deinit(&update_agent->fw_directory);
	stream_manager_deinit(&update_agent->stream_manager);
}

int update_agent_begin_staging(struct update_agent *update_agent)
{
	int status = FWU_STATUS_DENIED;

	/* If already staging, any previous installation state is discarded */
	update_agent_cancel_staging(update_agent);

	if (update_agent->state == FWU_STATE_REGULAR) {
		status = fw_store_begin_install(update_agent->fw_store);

		/* Check if ready to install images */
		if (status == FWU_STATUS_SUCCESS)
			update_agent->state = FWU_STATE_STAGING;
	}

	return status;
}

int update_agent_end_staging(struct update_agent *update_agent)
{
	int status = FWU_STATUS_DENIED;

	if (update_agent->state == FWU_STATE_STAGING) {
		/* The client is responsible for committing each installed image. If any
		 * install streams have been left open, not all images were committed.
		 */
		bool any_uncommitted = stream_manager_is_open_streams(&update_agent->stream_manager,
								      FWU_STREAM_TYPE_INSTALL);

		if (!any_uncommitted) {
			/* All installed images have been committed so we're
			 * ready for a trial.
			 */
			status = fw_store_finalize_install(update_agent->fw_store);

			if (status == FWU_STATUS_SUCCESS)
				/* Transition to TRAIL_PENDING state. The trial actually starts
				 * when installed images are activated through a system restart.
				 */
				update_agent->state = FWU_STATE_TRIAL_PENDING;

		} else {
			/* Client failed to commit all images installed */
			status = FWU_STATUS_BUSY;
		}
	}

	return status;
}

int update_agent_cancel_staging(struct update_agent *update_agent)
{
	int status = FWU_STATUS_DENIED;

	if (update_agent->state == FWU_STATE_STAGING) {
		stream_manager_cancel_streams(&update_agent->stream_manager,
					      FWU_STREAM_TYPE_INSTALL);

		fw_store_cancel_install(update_agent->fw_store);

		update_agent->state = FWU_STATE_REGULAR;

		status = FWU_STATUS_SUCCESS;
	}

	return status;
}

int update_agent_accept(struct update_agent *update_agent,
			const struct uuid_octets *image_type_uuid)
{
	int status = FWU_STATUS_DENIED;

	if (update_agent->state == FWU_STATE_TRIAL) {
		const struct image_info *image_info =
			fw_directory_find_image_info(&update_agent->fw_directory, image_type_uuid);

		if (image_info) {
			if (fw_store_notify_accepted(update_agent->fw_store, image_info)) {
				/* From the fw_store perspective, the update has
				 * been fully accepted.
				 */
				status = fw_store_commit_to_update(update_agent->fw_store);
				update_agent->state = FWU_STATE_REGULAR;

			} else
				/* Still more images to accept */
				status = FWU_STATUS_SUCCESS;
		} else
			/* Unrecognised image uuid */
			status = FWU_STATUS_UNKNOWN;
	}

	return status;
}

int update_agent_select_previous(struct update_agent *update_agent)
{
	int status = FWU_STATUS_DENIED;

	if ((update_agent->state == FWU_STATE_TRIAL) ||
	    (update_agent->state == FWU_STATE_TRIAL_PENDING)) {
		status = fw_store_revert_to_previous(update_agent->fw_store);
		update_agent->state = FWU_STATE_REGULAR;
	}

	return status;
}

int update_agent_open(struct update_agent *update_agent, const struct uuid_octets *uuid,
		      uint32_t *handle)
{
	int status;

	/* Pass UUID along a chain-of-responsibility until it's handled */
	if (!open_image_directory(update_agent, uuid, handle, &status) &&
	    !open_fw_store_object(update_agent, uuid, handle, &status) &&
	    !open_fw_image(update_agent, uuid, handle, &status)) {
		/* UUID not recognised */
		status = FWU_STATUS_UNKNOWN;
	}

	return status;
}

int update_agent_commit(struct update_agent *update_agent, uint32_t handle, bool accepted)
{
	return stream_manager_close(&update_agent->stream_manager, handle, accepted);
}

int update_agent_write_stream(struct update_agent *update_agent, uint32_t handle,
			      const uint8_t *data, size_t data_len)
{
	return stream_manager_write(&update_agent->stream_manager, handle, data, data_len);
}

int update_agent_read_stream(struct update_agent *update_agent, uint32_t handle, uint8_t *buf,
			     size_t buf_size, size_t *read_len, size_t *total_len)
{
	return stream_manager_read(&update_agent->stream_manager, handle, buf, buf_size, read_len,
				   total_len);
}

static bool open_image_directory(struct update_agent *update_agent, const struct uuid_octets *uuid,
				 uint32_t *handle, int *status)
{
	struct uuid_octets target_uuid;

	uuid_guid_octets_from_canonical(&target_uuid, FWU_DIRECTORY_CANONICAL_UUID);

	if (uuid_is_equal(uuid->octets, target_uuid.octets)) {
		/* Serialize a fresh view of the image directory */
		size_t serialized_len = 0;

		*status = img_dir_serializer_serialize(&update_agent->fw_directory,
						       update_agent->fw_store,
						       update_agent->image_dir_buf,
						       update_agent->image_dir_buf_size,
						       &serialized_len);

		if (*status == FWU_STATUS_SUCCESS) {
			*status = stream_manager_open_buffer_stream(&update_agent->stream_manager,
								    update_agent->image_dir_buf,
								    serialized_len, handle);
		}

		return true;
	}

	return false;
}

static bool open_fw_store_object(struct update_agent *update_agent, const struct uuid_octets *uuid,
				 uint32_t *handle, int *status)
{
	const uint8_t *exported_data;
	size_t exported_data_len;

	if (fw_store_export(update_agent->fw_store, uuid, &exported_data, &exported_data_len,
			    status)) {
		if (*status == FWU_STATUS_SUCCESS) {
			*status = stream_manager_open_buffer_stream(&update_agent->stream_manager,
								    exported_data,
								    exported_data_len, handle);
		}

		return true;
	}

	return false;
}

static bool open_fw_image(struct update_agent *update_agent, const struct uuid_octets *uuid,
			  uint32_t *handle, int *status)
{
	const struct image_info *image_info =
		fw_directory_find_image_info(&update_agent->fw_directory, uuid);

	if (image_info) {
		if (update_agent->state == FWU_STATE_STAGING) {
			struct installer *installer;

			*status = fw_store_select_installer(update_agent->fw_store, image_info,
							    &installer);

			if (*status == FWU_STATUS_SUCCESS) {
				*status = stream_manager_open_install_stream(
					&update_agent->stream_manager, update_agent->fw_store,
					installer, image_info, handle);
			}
		} else {
			/* Attempting to open a fw image when not staging */
			*status = FWU_STATUS_DENIED;
		}

		return true;
	}

	return false;
}

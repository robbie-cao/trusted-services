/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FW_UPDATE_AGENT_H
#define FW_UPDATE_AGENT_H

#include <stdbool.h>
#include <stdint.h>

#include "common/uuid/uuid.h"
#include "fw_directory.h"
#include "service/fwu/inspector/fw_inspector.h"
#include "stream_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interface dependencies
 */
struct fw_store;

/**
 * \brief Update process states
 *
 * The update_agent is responsible for ensuring that only a valid update flow
 * is followed by a client. To enforce the flow, public operations can only be
 * used in a valid state that reflects the FWU-A behavioral model.
 */
enum fwu_state {
	FWU_STATE_DEINITIALZED,
	FWU_STATE_INITIALIZING,
	FWU_STATE_REGULAR,
	FWU_STATE_STAGING,
	FWU_STATE_TRIAL_PENDING,
	FWU_STATE_TRIAL
};

/**
 * \brief update_agent structure definition
 *
 * An update_agent instance is responsible for coordinating firmware updates applied
 * to a fw_store. An update_agent performs a security role by enforcing that a
 * valid flow is performed to update the fw store.
 */
struct update_agent {
	enum fwu_state state;
	fw_inspector_inspect fw_inspect_method;
	struct fw_store *fw_store;
	struct fw_directory fw_directory;
	struct stream_manager stream_manager;
	uint8_t *image_dir_buf;
	size_t image_dir_buf_size;
};

/**
 * \brief Initialise the update_agent
 *
 * \param[in]  update_agent    The subject update_agent
 * \param[in]  boot_index      The boot_index used by the bootloader
 * \param[in]  fw_inspect_method  fw_inspector inspect method
 * \param[in]  fw_store        The fw_store to manage
 *
 * \return Status (0 for success).  Uses fwu protocol status codes.
 */
int update_agent_init(struct update_agent *update_agent, unsigned int boot_index,
		      fw_inspector_inspect fw_inspect_method, struct fw_store *fw_store);

/**
 * \brief De-initialise the update_agent
 *
 * \param[in]  update_agent    The subject update_agent
 */
void update_agent_deinit(struct update_agent *update_agent);

/**
 * \brief Begin staging
 *
 * \param[in]  update_agent    The subject update_agent
 *
 * \return 0 on successfully transitioning to the STAGING state
 */
int update_agent_begin_staging(struct update_agent *update_agent);

/**
 * \brief End staging
 *
 * \param[in]  update_agent    The subject update_agent
 *
 * \return 0 on successfully transitioning to the TRIAL state
 */
int update_agent_end_staging(struct update_agent *update_agent);

/**
 * \brief Cancel staging
 *
 * \param[in]  update_agent    The subject update_agent
 *
 * \return 0 on successfully transitioning to the REGULAR state
 */
int update_agent_cancel_staging(struct update_agent *update_agent);

/**
 * \brief Accept an updated image
 *
 * \param[in]  update_agent    The subject update_agent
 * \param[in]  image_type_uuid Identifies the image to accept
 *
 * \return Status (0 on success)
 */
int update_agent_accept(struct update_agent *update_agent,
			const struct uuid_octets *image_type_uuid);

/**
 * \brief Select previous version
 *
 *  Revert to a previous good version (if possible).
 *
 * \param[in]  update_agent    The subject update_agent
 *
 * \return Status (0 on success)
 */
int update_agent_select_previous(struct update_agent *update_agent);

/**
 * \brief Open a stream for accessing an fwu stream
 *
 * Used for reading or writing data for accessing images or other fwu
 * related objects.
 *
 * \param[in]  update_agent    The subject update_agent
 * \param[in]  uuid            Identifies the object to access
 * \param[out] handle          For subsequent read/write operations
 *
 * \return Status (0 on success)
 */
int update_agent_open(struct update_agent *update_agent, const struct uuid_octets *uuid,
		      uint32_t *handle);

/**
 * \brief Close a stream and commit any writes to the stream
 *
 * \param[in]  update_agent    The subject update_agent
 * \param[in]  handle          The handle returned by open
 * \param[in]  accepted        Initial accepted state of an image
 *
 * \return Status (0 on success)
 */
int update_agent_commit(struct update_agent *update_agent, uint32_t handle, bool accepted);

/**
 * \brief Write to a previously opened stream
 *
 * \param[in]  update_agent    The subject update_agent
 * \param[in]  handle          The handle returned by open
 * \param[in]  data            Pointer to data
 * \param[in]  data_len        The data length
 *
 * \return Status (0 on success)
 */
int update_agent_write_stream(struct update_agent *update_agent, uint32_t handle,
			      const uint8_t *data, size_t data_len);

/**
 * \brief Read from a previously opened stream
 *
 * \param[in]  update_agent    The subject update_agent
 * \param[in]  handle          The handle returned by open
 * \param[in]  buf             Pointer to buffer to copy to
 * \param[in]  buf_size        The size of the buffer
 * \param[out] read_len        The length of data read
 * \param[out] total_len       The total length of the object to read
 *
 * \return Status (0 on success)
 */
int update_agent_read_stream(struct update_agent *update_agent, uint32_t handle, uint8_t *buf,
			     size_t buf_size, size_t *read_len, size_t *total_len);

#ifdef __cplusplus
}
#endif

#endif /* FW_UPDATE_AGENT_H */

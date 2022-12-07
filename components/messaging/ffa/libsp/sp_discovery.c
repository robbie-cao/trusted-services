// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 */

#include "sp_discovery.h"
#include "ffa_api.h"
#include "sp_rxtx.h"
#include "util.h"
#include <string.h>

static const struct sp_uuid uuid_nil = { 0 };

sp_result sp_discovery_ffa_version_get(uint16_t *major, uint16_t *minor)
{
	uint32_t version = 0;
	ffa_result ffa_res = FFA_OK;

	ffa_res = ffa_version(&version);
	if (ffa_res != FFA_OK) {
		*major = UINT16_C(0);
		*minor = UINT16_C(0);

		return SP_RESULT_FFA(ffa_res);
	}

	*major = (version >> FFA_VERSION_MAJOR_SHIFT) & FFA_VERSION_MAJOR_MASK;
	*minor = (version >> FFA_VERSION_MINOR_SHIFT) & FFA_VERSION_MINOR_MASK;

	return SP_RESULT_OK;
}

sp_result sp_discovery_own_id_get(uint16_t *id)
{
	ffa_result ffa_res = FFA_OK;

	ffa_res = ffa_id_get(id);
	return SP_RESULT_FFA(ffa_res);
}

static void unpack_ffa_info(const struct ffa_partition_information *ffa_info,
			    struct sp_partition_info *sp_info)
{
	uint32_t props = ffa_info->partition_properties;

	sp_info->partition_id = ffa_info->partition_id;
	sp_info->execution_context_count = ffa_info->execution_context_count;
	sp_info->supports_direct_requests =
		props & FFA_PARTITION_SUPPORTS_DIRECT_REQUESTS;
	sp_info->can_send_direct_requests =
		props & FFA_PARTITION_CAN_SEND_DIRECT_REQUESTS;
	sp_info->supports_indirect_requests =
		props & FFA_PARTITION_SUPPORTS_INDIRECT_REQUESTS;
}

static sp_result
partition_info_get(const struct sp_uuid *uuid,
		   struct sp_partition_info info[],
		   uint32_t *count,
		   bool allow_nil_uuid)
{
	const struct ffa_partition_information *ffa_info = NULL;
	uint32_t ffa_count = 0;
	uint32_t i = 0;
	sp_result sp_res = SP_RESULT_OK;
	const void *buffer = NULL;
	size_t buffer_size = 0;
	struct ffa_uuid ffa_uuid = { 0 };
	ffa_result ffa_res = FFA_OK;

	if (count == NULL)
		return SP_RESULT_INVALID_PARAMETERS;

	if (info == NULL) {
		*count = UINT32_C(0);
		return SP_RESULT_INVALID_PARAMETERS;
	}

	if (uuid == NULL || (!allow_nil_uuid &&
	    memcmp(&uuid_nil, uuid, sizeof(struct sp_uuid)) == 0)) {
		sp_res = SP_RESULT_INVALID_PARAMETERS;
		goto out;
	}

	sp_res = sp_rxtx_buffer_rx_get(&buffer, &buffer_size);
	if (sp_res != SP_RESULT_OK) {
		goto out;
	}

	/* Safely convert to FF-A UUID format */
	memcpy(&ffa_uuid.uuid, uuid->uuid, sizeof(ffa_uuid.uuid));

	ffa_res = ffa_partition_info_get(&ffa_uuid, &ffa_count);
	if (ffa_res != FFA_OK) {
		sp_res = SP_RESULT_FFA(ffa_res);
		goto out;
	}

	if ((ffa_count * sizeof(struct ffa_partition_information)) > buffer_size) {
		/*
		 * The indicated amount of info structures doesn't fit into the
		 * RX buffer.
		 */
		sp_res = SP_RESULT_INTERNAL_ERROR;
		goto out;
	}

	ffa_info = (const struct ffa_partition_information *)buffer;

	if (ffa_count == 0) {
		sp_res = SP_RESULT_NOT_FOUND;
		goto out;
	}

	*count = MIN(*count, ffa_count);
	for (i = 0; i < *count; i++)
		unpack_ffa_info(&ffa_info[i], &info[i]);

	return SP_RESULT_OK;

out:
	for (i = 0; i < *count; i++)
		info[i] =  (struct sp_partition_info){ 0 };
	*count = UINT32_C(0);

	return sp_res;
}

sp_result sp_discovery_partition_id_get(const struct sp_uuid *uuid,
					uint16_t *id)
{
	struct sp_partition_info sp_info = { 0 };
	sp_result sp_res = SP_RESULT_OK;
	uint32_t count = 1;

	if (id == NULL)
		return SP_RESULT_INVALID_PARAMETERS;

	*id = FFA_ID_GET_ID_MASK;

	if (uuid == NULL || memcmp(&uuid_nil, uuid, sizeof(struct sp_uuid)) == 0)
		return SP_RESULT_INVALID_PARAMETERS;

	sp_res = partition_info_get(uuid, &sp_info, &count, false);
	if (sp_res != SP_RESULT_OK)
		return sp_res;

	*id = sp_info.partition_id;

	return SP_RESULT_OK;
}

sp_result sp_discovery_partition_info_get(const struct sp_uuid *uuid,
					  struct sp_partition_info info[],
					  uint32_t *count)
{
	return partition_info_get(uuid, info, count, false);
}

sp_result sp_discovery_partition_info_get_all(struct sp_partition_info info[],
					      uint32_t *count)
{
	return partition_info_get(&uuid_nil, info, count, true);
}

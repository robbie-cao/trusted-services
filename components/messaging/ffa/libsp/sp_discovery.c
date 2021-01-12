// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
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

static sp_result
partition_info_get(const struct sp_uuid *uuid,
		   const struct ffa_partition_information **info,
		   uint32_t *count)
{
	const void *buffer = NULL;
	size_t buffer_size = 0;
	struct ffa_uuid ffa_uuid = { 0 };
	sp_result sp_res = SP_RESULT_OK;
	ffa_result ffa_res = FFA_OK;

	sp_res = sp_rxtx_buffer_rx_get(&buffer, &buffer_size);
	if (sp_res != SP_RESULT_OK) {
		*count = UINT32_C(0);
		return sp_res;
	}

	/* Safely convert to FF-A UUID format */
	memcpy(&ffa_uuid.uuid, uuid->uuid, sizeof(ffa_uuid.uuid));

	ffa_res = ffa_partition_info_get(&ffa_uuid, count);
	if (ffa_res != FFA_OK) {
		*count = UINT32_C(0);
		return SP_RESULT_FFA(ffa_res);
	}

	if ((*count * sizeof(struct ffa_partition_information)) > buffer_size) {
		/*
		 * The indicated amount of info structures doesn't fit into the
		 * RX buffer.
		 */
		*count = UINT32_C(0);
		return SP_RESULT_INTERNAL_ERROR;
	}

	*info = (const struct ffa_partition_information *)buffer;

	return SP_RESULT_OK;
}

static sp_result
partition_info_get_single(const struct sp_uuid *uuid,
			  const struct ffa_partition_information **info)
{
	uint32_t count = 0;
	sp_result sp_res = SP_RESULT_OK;

	if (uuid == NULL)
		return SP_RESULT_INVALID_PARAMETERS;

	/*
	 * Nil UUID means querying all partitions which is handled by a separate
	 * function.
	 */
	if (memcmp(&uuid_nil, uuid, sizeof(struct sp_uuid)) == 0)
		return SP_RESULT_INVALID_PARAMETERS;

	sp_res = partition_info_get(uuid, info, &count);
	if (sp_res != SP_RESULT_OK)
		return sp_res;

	if (count == 0)
		return SP_RESULT_NOT_FOUND;

	return SP_RESULT_OK;
}

static void unpack_ffa_info(const struct ffa_partition_information ffa_info[],
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

sp_result sp_discovery_partition_id_get(const struct sp_uuid *uuid,
					uint16_t *id)
{
	const struct ffa_partition_information *ffa_info = NULL;
	uint32_t count = 0;
	sp_result sp_res = SP_RESULT_OK;

	if (id == NULL)
		return SP_RESULT_INVALID_PARAMETERS;

	sp_res = partition_info_get_single(uuid, &ffa_info);
	if (sp_res != SP_RESULT_OK) {
		*id = FFA_ID_GET_ID_MASK;
		return sp_res;
	}

	*id = ffa_info->partition_id;

	return SP_RESULT_OK;
}

sp_result sp_discovery_partition_info_get(const struct sp_uuid *uuid,
					  struct sp_partition_info *info)
{
	const struct ffa_partition_information *ffa_info = NULL;
	uint32_t count = 0;
	sp_result sp_res = SP_RESULT_OK;

	if (info == NULL)
		return SP_RESULT_INVALID_PARAMETERS;

	sp_res = partition_info_get_single(uuid, &ffa_info);
	if (sp_res != SP_RESULT_OK) {
		*info = (struct sp_partition_info){ 0 };
		return sp_res;
	}

	unpack_ffa_info(ffa_info, info);

	return SP_RESULT_OK;
}

sp_result sp_discovery_partition_info_get_all(struct sp_partition_info info[],
					      uint32_t *count)
{
	const struct ffa_partition_information *ffa_info = NULL;
	uint32_t ffa_count = 0;
	uint32_t i = 0;
	sp_result sp_res = SP_RESULT_OK;

	if (count == NULL)
		return SP_RESULT_INVALID_PARAMETERS;

	if (info == NULL) {
		*count = UINT32_C(0);
		return SP_RESULT_INVALID_PARAMETERS;
	}

	sp_res = partition_info_get(&uuid_nil, &ffa_info, &ffa_count);
	if (sp_res != SP_RESULT_OK) {
		*count = UINT32_C(0);
		return sp_res;
	}

	*count = MIN(*count, ffa_count);
	for (i = 0; i < *count; i++)
		unpack_ffa_info(&ffa_info[i], &info[i]);

	return SP_RESULT_OK;
}

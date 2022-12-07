/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LIBSP_INCLUDE_SP_DISCOVERY_H_
#define LIBSP_INCLUDE_SP_DISCOVERY_H_

#include "sp_api_defines.h"
#include "sp_api_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sp_uuid {
	uint8_t uuid[16];
};

struct sp_partition_info {
	uint16_t partition_id;
	uint16_t execution_context_count;
	bool supports_direct_requests;
	bool can_send_direct_requests;
	bool supports_indirect_requests;
};

/**
 * @brief       Queries the FF-A version of the FF-A instance.
 *
 * @param[out]  major  The major FF-A version
 * @param[out]  minor  The minor FF-A version
 *
 * @return      The SP API result
 */
sp_result sp_discovery_ffa_version_get(uint16_t *major, uint16_t *minor);

/**
 * @brief       Queries the 16 bit FF-A ID of the calling partition.
 *
 * @param[out]  id    The 16 bit FF-A ID of the calling partition
 *
 * @return      The SP API result
 */
sp_result sp_discovery_own_id_get(uint16_t *id);

/**
 * @brief       Queries the 16 bit FF-A ID of a partition by its UUID.
 *
 * @param[in]   uuid  The UUID of the partition
 * @param[out]  id    The 16 bit FF-A ID of the partition
 *
 * @return      The SP API result
 */
sp_result sp_discovery_partition_id_get(const struct sp_uuid *uuid,
					uint16_t *id);

/**
 * @brief       Queries the information about a partition by its UUID.
 *
 * @param[in]      uuid  The UUID of the partition
 * @param[out]     info  The partition information
 * @param[in,out]  count As an input value it specifies the count of partition
 *                       info structures that would fit into the output buffer.
 *                       As an output it indicates the count of the valid
 *                       entries in the buffer.
 *
 * @return         The SP API result
 */
sp_result sp_discovery_partition_info_get(const struct sp_uuid *uuid,
					  struct sp_partition_info *info,
					  uint32_t *count);

/**
 * @brief          Queries partition information of all partitions.
 *
 * @param[out]     info   The partition information buffer
 * @param[in,out]  count  As an input value it specifies the count of partition
 *                        info structures that would fit into the output buffer.
 *                        As an output it indicates the count of the valid
 *                        entries in the buffer.
 *
 * @return         The SP API result
 */
sp_result sp_discovery_partition_info_get_all(struct sp_partition_info info[],
					      uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif /* LIBSP_INCLUDE_SP_DISCOVERY_H_ */

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LIBSP_INCLUDE_FFA_API_TYPES_H_
#define LIBSP_INCLUDE_FFA_API_TYPES_H_

#include "compiler.h"
#include <stddef.h>
#include <stdint.h>

/**
 * Init info
 */

/**
 * @brief Boot protocol name-value pairs
 */
struct ffa_name_value_pair {
	uint32_t name[4]; /**< Name of the item */
	uintptr_t value; /**< Value of the item */
	size_t size; /**< Size of the referenced value */
};

/**
 * @brief Structure for passing boot protocol data
 */
struct ffa_init_info {
	uint32_t magic; /**< FF-A */
	uint32_t count; /**< Count of name value size pairs */
	struct ffa_name_value_pair nvp[]; /**< Array of name value size pairs */
};

/**
 * @brief FF-A error status code type
 */
typedef int32_t ffa_result;

/**
 * FF-A features types
 */

/**
 * @brief Used to encode any optional features implemented or any implementation
 *        details exported by the queried interface.
 */
struct ffa_interface_properties {
	uint32_t interface_properties[2];
};

/**
 * Partition information types
 */

/**
 * @brief UUID descriptor structure
 */
struct ffa_uuid {
	uint8_t uuid[16];
};

/**
 * @brief Table 8.25: Partition information descriptor
 */
struct ffa_partition_information {
	uint16_t partition_id;
	uint16_t execution_context_count;
	uint32_t partition_properties;
} __packed;

/**
 * Direct message type
 */

/**
 * @brief Direct message type
 */
struct ffa_direct_msg {
	uint32_t function_id;
	uint16_t source_id;
	uint16_t destination_id;
	uint32_t args[5];
};

#endif /* LIBSP_INCLUDE_FFA_API_TYPES_H_ */

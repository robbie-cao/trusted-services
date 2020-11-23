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

/**
 * Memory management transaction types
 */

/**
 * @brief Table 5.14: Constituent memory region descriptor
 */
struct ffa_constituent_mem_region_desc {
	uint64_t address;
	uint32_t page_count;
	uint32_t reserved_mbz;
} __packed;

/**
 * @brief Table 5.13: Composite memory region descriptor
 */
struct ffa_composite_mem_region_desc {
	uint32_t total_page_count;
	uint32_t address_range_count;
	uint64_t reserved_mbz;
	struct ffa_constituent_mem_region_desc constituent_mem_region_desc[];
} __packed;

/**
 * @brief Table 5.15: Memory access permissions descriptor
 */
struct ffa_mem_access_perm_desc {
	uint16_t endpoint_id;
	uint8_t mem_access_permissions;
	uint8_t flags;
} __packed;

/**
 * @brief Table 5.16: Endpoint memory access descriptor
 */
struct ffa_mem_access_desc {
	struct ffa_mem_access_perm_desc mem_access_perm_desc;
	uint32_t composite_mem_region_desc_offset;
	uint64_t reserved_mbz;
} __packed;

/**
 * @brief Table 5.19: Lend, donate or share memory transaction descriptor
 */
struct ffa_mem_transaction_desc {
	uint16_t sender_id;
	uint8_t mem_region_attr;
	uint8_t reserved_mbz0;
	uint32_t flags;
	uint64_t handle;
	uint64_t tag;
	uint32_t reserved_mbz1;
	uint32_t mem_access_desc_count;
	struct ffa_mem_access_desc mem_access_desc[];
} __packed;

/**
 * @brief Table 11.25: Descriptor to relinquish a memory region
 */
struct ffa_mem_relinquish_desc {
	uint64_t handle;
	uint32_t flags;
	uint32_t endpoint_count;
	uint16_t endpoints[];
} __packed;

#endif /* LIBSP_INCLUDE_FFA_API_TYPES_H_ */

/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_STORAGE_PROTO_H
#define SECURE_STORAGE_PROTO_H

#include <stdint.h>

struct __attribute__ ((__packed__)) secure_storage_request_set {
	uint64_t uid;
	uint64_t data_length;
	uint32_t create_flags;
	uint8_t p_data[];
};

struct __attribute__ ((__packed__)) secure_storage_request_get {
	uint64_t uid;
	uint64_t data_offset;
	uint64_t data_size;
};

struct __attribute__ ((__packed__)) secure_storage_request_get_info {
	uint64_t uid;
};

struct __attribute__ ((__packed__)) secure_storage_response_get_info {
	uint64_t capacity;
	uint64_t size;
	uint32_t flags;
};

struct __attribute__ ((__packed__)) secure_storage_request_remove {
	uint64_t uid;
};

#define TS_SECURE_STORAGE_OPCODE_SET			(0u)
#define TS_SECURE_STORAGE_OPCODE_GET			(1u)
#define TS_SECURE_STORAGE_OPCODE_GET_INFO		(2u)
#define TS_SECURE_STORAGE_OPCODE_REMOVE			(3u)

#define TS_SECURE_STORAGE_FLAG_NONE			(0u)
#define TS_SECURE_STORAGE_FLAG_WRITE_ONCE		(1u << 0)
#define TS_SECURE_STORAGE_FLAG_NO_CONFIDENTIALITY	(1u << 1)
#define TS_SECURE_STORAGE_FLAG_NO_REPLAY_PROTECTION	(1u << 2)
#define TS_SECURE_STORAGE_SUPPORT_SET_EXTENDED		(1u << 0)

#endif /* SECURE_STORAGE_PROTO_H */

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021-2022, Arm Limited.
 */

#ifndef OPTEE_STORAGE_BACKEND_H_
#define OPTEE_STORAGE_BACKEND_H_

#include "service/secure_storage/backend/storage_backend.h"
#include "tee_api_defines_extensions.h"

/**
 * @brief Initializes the storage backend according to the selected storage type
 *
 * @param storage_id TEE_STORAGE_PRIVATE_REE or TEE_STORAGE_PRIVATE_RPMB
 * @return struct storage_backend * Storage backend descriptor
 */
struct storage_backend *optee_storage_backend_init(unsigned long storage_id);

/**
 * @brief Allows the caller to assign a shared buffer which can be used to
 * allocate variables in the storage system which has access from the SEL0 SP.
 * This is necessary to pass the MMU checks.
 *
 * @param buffer Address of the buffer
 * @param buffer_size  Buffer size
 */
void optee_storage_backend_assign_shared_buffer(struct storage_backend *backend,
						void *buffer,
						size_t buffer_size);

#endif /* OPTEE_STORAGE_BACKEND_H_ */

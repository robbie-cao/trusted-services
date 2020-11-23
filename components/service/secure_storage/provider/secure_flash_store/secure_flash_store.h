/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __SECURE_FLASH_STORE_H__
#define __SECURE_FLASH_STORE_H__

#include <stddef.h>
#include <stdint.h>

#include <protocols/service/psa/packed-c/status.h>
#include <protocols/service/secure_storage/packed-c/secure_storage_proto.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initializes the internal trusted storage system.
 *
 * \return A status indicating the success/failure of the operation
 *
 * \retval PSA_SUCCESS                The operation completed successfully
 * \retval PSA_ERROR_STORAGE_FAILURE  The operation failed because the storage
 *                                    system initialization has failed (fatal
 *                                    error)
 * \retval PSA_ERROR_GENERIC_ERROR    The operation failed because of an
 *                                    unspecified internal failure
 */
psa_status_t sfs_init(void);

/**
 * \brief Create a new, or modify an existing, uid/value pair
 *
 * Stores data in the internal storage.
 *
 * \param[in] client_id     Identifier of the asset's owner (client)
 * \param[in] uid           The identifier for the data
 * \param[in] data_length   The size in bytes of the data in `p_data`
 * \param[in] create_flags  The flags that the data will be stored with
 *
 * \return A status indicating the success/failure of the operation
 *
 * \retval PSA_SUCCESS                     The operation completed successfully
 * \retval PSA_ERROR_NOT_PERMITTED         The operation failed because the
 *                                         provided `uid` value was already
 *                                         created with
 *                                         TS_SECURE_STORAGE_FLAG_WRITE_ONCE
 * \retval PSA_ERROR_NOT_SUPPORTED         The operation failed because one or
 *                                         more of the flags provided in
 *                                         `create_flags` is not supported or is
 *                                         not valid
 * \retval PSA_ERROR_INSUFFICIENT_STORAGE  The operation failed because there
 *                                         was insufficient space on the
 *                                         storage medium
 * \retval PSA_ERROR_STORAGE_FAILURE       The operation failed because the
 *                                         physical storage has failed (Fatal
 *                                         error)
 * \retval PSA_ERROR_INVALID_ARGUMENT      The operation failed because one
 *                                         of the provided pointers (`p_data`)
 *                                         is invalid, for example is `NULL` or
 *                                         references memory the caller cannot
 *                                         access
 */
psa_status_t sfs_set(uint32_t client_id,
                         uint64_t uid,
                         size_t data_length,
                         const void *p_data,
                         uint32_t create_flags);

/**
 * \brief Retrieve data associated with a provided UID
 *
 * Retrieves up to `data_size` bytes of the data associated with `uid`, starting
 * at `data_offset` bytes from the beginning of the data. Upon successful
 * completion, the data will be placed in the `p_data` buffer, which must be at
 * least `data_size` bytes in size. The length of the data returned will be in
 * `p_data_length`. If `data_size` is 0, the contents of `p_data_length` will
 * be set to zero.
 *
 * \param[in]  client_id      Identifier of the asset's owner (client)
 * \param[in]  uid            The uid value
 * \param[in]  data_offset    The starting offset of the data requested
 * \param[in]  data_size      The amount of data requested
 * \param[out] p_data_length  On success, this will contain size of the data
 *                            placed in `p_data`.
 *
 * \return A status indicating the success/failure of the operation
 *
 * \retval PSA_SUCCESS                 The operation completed successfully
 * \retval PSA_ERROR_DOES_NOT_EXIST    The operation failed because the
 *                                     provided `uid` value was not found in
 *                                     the storage
 * \retval PSA_ERROR_STORAGE_FAILURE   The operation failed because the
 *                                     physical storage has failed (Fatal
 *                                     error)
 * \retval PSA_ERROR_INVALID_ARGUMENT  The operation failed because one of the
 *                                     provided arguments (`p_data`,
 *                                     `p_data_length`) is invalid, for example
 *                                     is `NULL` or references memory the
 *                                     caller cannot access. In addition, this
 *                                     can also happen if `data_offset` is
 *                                     larger than the size of the data
 *                                     associated with `uid`.
 */
psa_status_t sfs_get(uint32_t client_id,
                         uint64_t uid,
                         size_t data_offset,
                         size_t data_size,
                         void *p_data,
                         size_t *p_data_length);

/**
 * \brief Retrieve the metadata about the provided uid
 *
 * Retrieves the metadata stored for a given `uid` as a `secure_storage_response_get_info`
 * structure.
 *
 * \param[in]  client_id  Identifier of the asset's owner (client)
 * \param[in]  uid        The `uid` value
 * \param[out] p_info     A pointer to the `secure_storage_response_get_info` struct that will
 *                        be populated with the metadata
 *
 * \return A status indicating the success/failure of the operation
 *
 * \retval PSA_SUCCESS                 The operation completed successfully
 * \retval PSA_ERROR_DOES_NOT_EXIST    The operation failed because the provided
 *                                     uid value was not found in the storage
 * \retval PSA_ERROR_STORAGE_FAILURE   The operation failed because the physical
 *                                     storage has failed (Fatal error)
 * \retval PSA_ERROR_INVALID_ARGUMENT  The operation failed because one of the
 *                                     provided pointers(`p_info`)
 *                                     is invalid, for example is `NULL` or
 *                                     references memory the caller cannot
 *                                     access
 */
psa_status_t sfs_get_info(uint32_t client_id, uint64_t uid,
                              struct secure_storage_response_get_info *p_info);

/**
 * \brief Remove the provided uid and sfs associated data from the storage
 *
 * Deletes the data from internal storage.
 *
 * \param[in] client_id  Identifier of the asset's owner (client)
 * \param[in] uid        The `uid` value
 *
 * \return A status indicating the success/failure of the operation
 *
 * \retval PSA_SUCCESS                 The operation completed successfully
 * \retval PSA_ERROR_INVALID_ARGUMENT  The operation failed because one or more
 *                                     of the given arguments were invalid (null
 *                                     pointer, wrong flags and so on)
 * \retval PSA_ERROR_DOES_NOT_EXIST    The operation failed because the provided
 *                                     uid value was not found in the storage
 * \retval PSA_ERROR_NOT_PERMITTED     The operation failed because the provided
 *                                     uid value was created with
 *                                     TS_SECURE_STORAGE_FLAG_WRITE_ONCE
 * \retval PSA_ERROR_STORAGE_FAILURE   The operation failed because the physical
 *                                     storage has failed (Fatal error)
 */
psa_status_t sfs_remove(uint32_t client_id, uint64_t uid);

#ifdef __cplusplus
}
#endif

#endif /* __SECURE_FLASH_STORE_H__ */

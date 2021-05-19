/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ATTEST_KEY_MNGR_H
#define ATTEST_KEY_MNGR_H

#include <stdbool.h>
#include <stddef.h>
#include <psa/crypto.h>

/* Key ID for a volatile IAK (for test) */
#define ATTEST_KEY_MNGR_VOLATILE_IAK            (0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The attestation key manager manages creation and access
 * to the IAK. In real device deployments, the IAK will
 * either be provisioned during manufacture or generated
 * on first run.  To accommodate both sceanrios and to support
 * testing without a persistent key store, the IAK is
 * genarated automatically if the corresponding persistent
 * key doesn't exist.
 */

/**
 * \brief Initialize the attest_key_mngr
 *
 * Initializes the attest_key_mngr.  The provided key id should
 * be used as the identifier for the IAK.  If a key ID of zero
 * is passed, a volatile IAK will be generated.  This is useful
 * for test purposes.
 *
 * \param[in] iak_id    The key id for the IAK
 */
void attest_key_mngr_init(psa_key_id_t iak_id);

/**
 * \brief De-initialize the attest_key_mngr
 */
void attest_key_mngr_deinit(void);

/**
 * \brief Get the IAK key handle
 *
 *  If an IAK doesn't exist, one will be generated.  This supports the
 *  generate-on-first-run strategy.
 *
 * \param[out] iak_handle  The returned key handle
 * \return Status
 */
psa_status_t attest_key_mngr_get_iak_handle(psa_key_handle_t *iak_handle);

/**
 * \brief Export the IAK public key
 *
 *  Like the above method, if no IAK exists, one will be generated.
 *
 * \param[out] data  Buffer for key data
 * \param[in]  data_size Size of buffer
 * \param[out] data_length  Length in bytes of key
 *
 * \return Status
 */
psa_status_t attest_key_mngr_export_iak_public_key(uint8_t *data,
                                size_t data_size, size_t *data_length);

/**
 * \brief Import the IAK key-pair
 *
 * \param[in]  data  The key data
 * \param[out] data_length  Length in bytes of the key-pair
 *
 * \return Status
 */
psa_status_t attest_key_mngr_import_iak(const uint8_t *data, size_t data_length);

/**
 * \brief Check if the IAK exists
 *
 * \return True if IAK exist
 */
bool attest_key_mngr_iak_exists(void);

/**
 * \brief Return maximum size of an exported IAK public key
 *
 * \return Maximum export size
 */
size_t attest_key_mngr_max_iak_export_size(void);

/**
 * \brief Return maximum size of an imported IAK key
 *
 * \return Maximum import size
 */
size_t attest_key_mngr_max_iak_import_size(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ATTEST_KEY_MNGR_H */

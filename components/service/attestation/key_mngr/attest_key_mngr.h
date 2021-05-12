/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ATTEST_KEY_MNGR_H
#define ATTEST_KEY_MNGR_H

#include <psa/crypto.h>

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
 */
void attest_key_mngr_init(void);

/**
 * \brief De-initialize the attest_key_mngr
 */
void attest_key_mngr_deinit(void);

/**
 * \brief Get the IAK key handle
 *
 * \param[out] iak_handle  The returned key handle
 * \return Status
 */
psa_status_t attest_key_mngr_get_iak_handle(psa_key_handle_t *iak_handle);

/**
 * \brief Export the IAK public key
 *
 * \param[out] data  Buffer for key data
 * \param[in]  data_size Size of buffer
 * \param[out] data_length  Length in bytes of key
 *
 * \return Status
 */
psa_status_t attest_key_mngr_export_iak_public_key(uint8_t *data,
                                size_t data_size, size_t *data_length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ATTEST_KEY_MNGR_H */

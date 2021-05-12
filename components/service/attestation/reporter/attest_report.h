/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ATTEST_REPORT_H
#define ATTEST_REPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <psa/crypto.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates an attestation report
 *
 *  Using the view of the security state of the device provided by
 *  the claims_register, a signed attestation report is created.  On
 *  success, a buffer is allocated for the signed report.  The buffer
 *  must be freed by calling attest_report_destroy().
 *
 * \param[in] key_handle            Signing key handle
 * \param[in] client_id             The requesting client id
 * \param[in] auth_challenge_data   The auth challenge from the requester
 * \param[in] auth_challenge_len    The auth challenge from the requester
 * \param[out] report               The created report
 * \param[out] report_len           The length of the report
 *
 * \return Operation status
 */
int attest_report_create(psa_key_handle_t key_handle, int32_t client_id,
        const uint8_t *auth_challenge_data, size_t auth_challenge_len,
        const uint8_t **report, size_t *report_len);

/**
 * \brief Destroys an attestation report
 *
 *  Frees any resource associated with a created report
 *
 * \param[in] report               The created report
 */
void attest_report_destroy(const uint8_t *report);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ATTEST_REPORT_H */

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSA_REPORT_SERIALIZER_H
#define PSA_REPORT_SERIALIZER_H

#include <stddef.h>
#include <stdint.h>
#include <service/attestation/claims/claim_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Serialize the collated set of claims
 *
 *  Serializes the claims into a CBOR document using PSA defined
 *  EAT custom claims to identify claim objects.
 *
 * \param[in] device_claims         Collated device claims
 * \param[in] sw_claims             Collated software claims
 * \param[out] report               The serialized report
 * \param[out] report_len           The length of the report
 *
 * \return Operation status
 */
int serialize_report(const struct claim_vector *device_claims,
    const struct claim_vector *sw_claims,
    const uint8_t **report, size_t *report_len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PSA_REPORT_SERIALIZER_H */

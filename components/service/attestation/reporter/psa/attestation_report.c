/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * An attestation reporter that creates PSA compliant attestation
 * reports.  The report content is specified by theh PSA Attestation
 * specification.  Reports are serialized usingg CBOR and signed using
 * COSE.
 */

#include <stdlib.h>
#include <psa/error.h>
#include <service/attestation/reporter/attestation_report.h>
#include <service/attestation/claims/claims_register.h>
#include "report_serializer.h"

/* Local defines */
#define MAX_DEVICE_CLAIMS       (50)
#define MAX_SW_CLAIMS           (50)

static void add_auth_challenge_claim(struct claim_vector *v, const uint8_t *data, size_t len);
static void add_client_id_claim(struct claim_vector *v, int32_t client_id);
static void add_no_sw_claim(struct claim_vector *v);


int attestation_report_create(int32_t client_id,
    const uint8_t *auth_challenge_data, size_t auth_challenge_len,
    const uint8_t **report, size_t *report_len)
{
    psa_status_t status = PSA_ERROR_GENERIC_ERROR;
    struct claim_vector device_claims;
    struct claim_vector sw_claims;

    *report = NULL;
    *report_len = 0;

    claim_vector_init(&device_claims, MAX_DEVICE_CLAIMS);
    claim_vector_init(&sw_claims, MAX_SW_CLAIMS);

    /* Add claims related to the requester */
    add_auth_challenge_claim(&device_claims, auth_challenge_data, auth_challenge_len);
    add_client_id_claim(&device_claims, client_id);

    /* Collate all other claims to include in the report */
    claims_register_query_by_category(CLAIM_CATEGORY_DEVICE, &device_claims);
    claims_register_query_by_category(CLAIM_CATEGORY_VERIFICATION_SERVICE, &device_claims);
    claims_register_query_by_category(CLAIM_CATEGORY_BOOT_MEASUREMENT, &sw_claims);

    /* And if there aren't any sw claims, indicate in report */
    if (!sw_claims.size) add_no_sw_claim(&device_claims);

    /* Serialize the collated claims to create the final report */
    status = serialize_report(&device_claims, &sw_claims, report, report_len);

    claim_vector_deinit(&device_claims);
    claim_vector_deinit(&sw_claims);

    return status;
}

void attestation_report_destroy(const uint8_t *report)
{
    free((void*)report);
}

static void add_auth_challenge_claim(struct claim_vector *v, const uint8_t *data, size_t len)
{
    struct claim claim;

    claim.subject_id = CLAIM_SUBJECT_ID_AUTH_CHALLENGE;
    claim.variant_id = CLAIM_VARIANT_ID_BYTE_STRING;
    claim.raw_data = NULL;

    claim.variant.byte_string.bytes = data;
    claim.variant.byte_string.len = len;

    claim_vector_push_back(v, &claim);
}

static void add_client_id_claim(struct claim_vector *v, int32_t client_id)
{
    struct claim claim;

    claim.subject_id = CLAIM_SUBJECT_ID_CLIENT_ID;
    claim.variant_id = CLAIM_VARIANT_ID_INTEGER;
    claim.raw_data = NULL;

    claim.variant.integer.value = client_id;

    claim_vector_push_back(v, &claim);
}

static void add_no_sw_claim(struct claim_vector *v)
{
    struct claim claim;

    claim.subject_id = CLAIM_SUBJECT_ID_NO_SW_MEASUREMENTS;
    claim.variant_id = CLAIM_VARIANT_ID_INTEGER;
    claim.raw_data = NULL;

    claim.variant.integer.value = 1;

    claim_vector_push_back(v, &claim);
}

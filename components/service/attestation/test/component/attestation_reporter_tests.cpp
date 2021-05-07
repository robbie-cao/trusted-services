/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <psa/error.h>
#include <qcbor/qcbor_spiffy_decode.h>
#include <service/attestation/claims/claims_register.h>
#include <service/attestation/claims/sources/event_log/event_log_claim_source.h>
#include <service/attestation/claims/sources/event_log/mock/mock_event_log.h>
#include <service/attestation/claims/sources/preloaded/preloaded_claim_source.h>
#include <service/attestation/reporter/attestation_report.h>
#include <protocols/service/attestation/packed-c/eat.h>
#include <CppUTest/TestHarness.h>
#include "report_dump.h"

#include <stdio.h>

TEST_GROUP(AttestationReporterTests)
{
    void setup()
    {
        struct claim_source *claim_source;

        report = NULL;
        report_len;

        /* The set of registered claim_sources determines the content
         * of a generated attestation source.  The set and type of
         * claim_sources registered will be deployment specific.
         */
        claims_register_init();

        /* Boot measurement source */
        claim_source = event_log_claim_source_init(&event_log_claim_source,
            mock_event_log_start(), mock_event_log_size());
        claims_register_add_claim_source(CLAIM_CATEGORY_BOOT_MEASUREMENT, claim_source);
    }

    void teardown()
    {
        attestation_report_destroy(report);
        claims_register_deinit();
    }

    struct event_log_claim_source event_log_claim_source;
    const uint8_t *report;
    size_t report_len;
};

TEST(AttestationReporterTests, createReport)
{
    int status;

    /* Client inputs */
    int32_t client_id = 0x552791aa;
    const uint8_t auth_challenge[] = {
         1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,
        17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
    };

    /* Create a report */
    status = attestation_report_create(client_id,
        auth_challenge, sizeof(auth_challenge),
        &report, &report_len);

    /* Expect the operation to succeed and a non-zero length
     * report created.
     */
    UNSIGNED_LONGS_EQUAL(PSA_SUCCESS, status);
    CHECK_TRUE(report);
    CHECK_TRUE(report_len);

    /* Check the report contents */
    QCBORDecodeContext decode_ctx;
    UsefulBufC report_buf;

    report_buf.ptr = report;
    report_buf.len = report_len;

    QCBORDecode_Init(&decode_ctx, report_buf, QCBOR_DECODE_MODE_NORMAL);
    QCBORDecode_EnterMap(&decode_ctx, NULL);

    /* Check client id */
    int64_t decoded_client_id = 0;
    QCBORDecode_GetInt64InMapN(&decode_ctx,
        EAT_ARM_PSA_CLAIM_ID_CLIENT_ID, &decoded_client_id);

    UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, QCBORDecode_GetError(&decode_ctx));
    UNSIGNED_LONGS_EQUAL(client_id, decoded_client_id);

    /* Check the auth challenge */
    UsefulBufC auth_challenge_buf;
    auth_challenge_buf.ptr = NULL;
    auth_challenge_buf.len = 0;
    QCBORDecode_GetByteStringInMapN(&decode_ctx,
        EAT_ARM_PSA_CLAIM_ID_CHALLENGE, &auth_challenge_buf);

    UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, QCBORDecode_GetError(&decode_ctx));
    CHECK_TRUE(auth_challenge_buf.ptr);
    UNSIGNED_LONGS_EQUAL(sizeof(auth_challenge), auth_challenge_buf.len);
    MEMCMP_EQUAL(auth_challenge, auth_challenge_buf.ptr, sizeof(auth_challenge));

    /* Shouldn't expect to see the 'NO_SW_COMPONENTS' claim */
    int64_t no_sw = 0;
    QCBORDecode_GetInt64InMapN(&decode_ctx, EAT_ARM_PSA_CLAIM_ID_NO_SW_COMPONENTS, &no_sw);
    UNSIGNED_LONGS_EQUAL(QCBOR_ERR_LABEL_NOT_FOUND, QCBORDecode_GetAndResetError(&decode_ctx));
    CHECK_FALSE(no_sw);

    /* Check the sw components */
    QCBORDecode_EnterArrayFromMapN(&decode_ctx, EAT_ARM_PSA_CLAIM_ID_SW_COMPONENTS);
    UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, QCBORDecode_GetError(&decode_ctx));

    /* Iterate over all array members */
    size_t sw_component_count = 0;
    while (true) {

        QCBORDecode_EnterMap(&decode_ctx, NULL);

        if (QCBORDecode_GetAndResetError(&decode_ctx) == QCBOR_SUCCESS) {

            CHECK_TRUE(sw_component_count < mock_event_Log_measurement_count());

            UsefulBufC property;
            const struct mock_event_log_measurement *measurement =
                mock_event_Log_measurement(sw_component_count);

            /* Check measurement id */
            QCBORDecode_GetByteStringInMapN(&decode_ctx,
                    EAT_SW_COMPONENT_CLAIM_ID_MEASUREMENT_TYPE, &property);
            UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, QCBORDecode_GetError(&decode_ctx));
            CHECK_TRUE(property.ptr);
            CHECK_TRUE(property.len);
            MEMCMP_EQUAL(measurement->id, property.ptr, property.len);

            /* Check measurement digest */
            QCBORDecode_GetByteStringInMapN(&decode_ctx,
                    EAT_SW_COMPONENT_CLAIM_ID_MEASUREMENT_VALUE, &property);
            UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, QCBORDecode_GetError(&decode_ctx));
            CHECK_TRUE(property.ptr);
            CHECK_TRUE(property.len);
            MEMCMP_EQUAL(measurement->digest, property.ptr, property.len);

            QCBORDecode_ExitMap(&decode_ctx);

            ++sw_component_count;
        }
        else {
            /* No more sw components */
            break;
        }
    }

    QCBORDecode_ExitArray(&decode_ctx);
    UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, QCBORDecode_GetError(&decode_ctx));

    QCBORError qcbor_error;
    QCBORDecode_ExitMap(&decode_ctx);
    qcbor_error = QCBORDecode_Finish(&decode_ctx);
    UNSIGNED_LONGS_EQUAL(QCBOR_SUCCESS, qcbor_error);
}

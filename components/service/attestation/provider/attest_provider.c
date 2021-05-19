/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <string.h>
#include <protocols/service/attestation/packed-c/opcodes.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <service/attestation/key_mngr/attest_key_mngr.h>
#include <service/attestation/reporter/attest_report.h>
#include <psa/initial_attestation.h>
#include "attest_provider.h"

/* Service request handlers */
static rpc_status_t get_token_handler(void *context, struct call_req* req);
static rpc_status_t get_token_size_handler(void *context, struct call_req* req);
static rpc_status_t export_iak_public_key_handler(void *context, struct call_req* req);
static rpc_status_t import_iak_handler(void *context, struct call_req* req);
static rpc_status_t iak_exists_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
    {TS_ATTESTATION_OPCODE_GET_TOKEN,               get_token_handler},
    {TS_ATTESTATION_OPCODE_GET_TOKEN_SIZE,          get_token_size_handler},
    {TS_ATTESTATION_OPCODE_EXPORT_IAK_PUBLIC_KEY,   export_iak_public_key_handler},
    {TS_ATTESTATION_OPCODE_IMPORT_IAK,              import_iak_handler},
    {TS_ATTESTATION_OPCODE_IAK_EXISTS,              iak_exists_handler}
};

struct rpc_interface *attest_provider_init(struct attest_provider *context, psa_key_id_t iak_id)
{
    struct rpc_interface *rpc_interface = NULL;

    if (context) {

        for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
            context->serializers[encoding] = NULL;

        service_provider_init(&context->base_provider, context,
                    handler_table, sizeof(handler_table)/sizeof(struct service_handler));

        attest_key_mngr_init(iak_id);

        rpc_interface = service_provider_get_rpc_interface(&context->base_provider);
    }

    return rpc_interface;
}

void attest_provider_deinit(struct attest_provider *context)
{
    (void)context;
    attest_key_mngr_deinit();
}

void attest_provider_register_serializer(struct attest_provider *context,
                unsigned int encoding, const struct attest_provider_serializer *serializer)
{
    if (encoding < TS_RPC_ENCODING_LIMIT)
        context->serializers[encoding] = serializer;
}

static const struct attest_provider_serializer *get_attest_serializer(
                struct attest_provider *context, const struct call_req *req)
{
    const struct attest_provider_serializer *serializer = NULL;
    unsigned int encoding = call_req_get_encoding(req);

    if (encoding < TS_RPC_ENCODING_LIMIT) serializer = context->serializers[encoding];

    return serializer;
}

static rpc_status_t get_token_handler(void *context, struct call_req* req)
{
    struct attest_provider *this_instance = (struct attest_provider*)context;
    rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

    uint8_t challenge[PSA_INITIAL_ATTEST_CHALLENGE_SIZE_64];
    size_t challenge_len = sizeof(challenge);

    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct attest_provider_serializer *serializer = get_attest_serializer(this_instance, req);

    if (serializer)
        rpc_status = serializer->deserialize_get_token_req(req_buf, challenge, &challenge_len);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_key_handle_t iak_handle;
        int opstatus = attest_key_mngr_get_iak_handle(&iak_handle);

        if (opstatus == PSA_SUCCESS) {

            const uint8_t *token = NULL;
            size_t token_size = 0;

            opstatus = attest_report_create(iak_handle,
                (int32_t)call_req_get_caller_id(req),
                challenge, challenge_len,
                &token, &token_size);

            if (opstatus == PSA_SUCCESS) {

                struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                rpc_status = serializer->serialize_get_token_resp(resp_buf, token, token_size);
            }

            attest_report_destroy(token);
        }

        call_req_set_opstatus(req, opstatus);
    }

    return rpc_status;
}

static rpc_status_t get_token_size_handler(void *context, struct call_req* req)
{
    struct attest_provider *this_instance = (struct attest_provider*)context;
    rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;

    uint8_t challenge[PSA_INITIAL_ATTEST_CHALLENGE_SIZE_64];
    size_t challenge_len = sizeof(challenge);

    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct attest_provider_serializer *serializer = get_attest_serializer(this_instance, req);

    memset(challenge, 0, sizeof(challenge));

    if (serializer)
        rpc_status = serializer->deserialize_get_token_size_req(req_buf, &challenge_len);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        psa_key_handle_t iak_handle;
        int opstatus = attest_key_mngr_get_iak_handle(&iak_handle);

        if (opstatus == PSA_SUCCESS) {

            const uint8_t *token = NULL;
            size_t token_size = 0;

            opstatus = attest_report_create(iak_handle,
                (int32_t)call_req_get_caller_id(req),
                challenge, challenge_len,
                &token, &token_size);

            if (opstatus == PSA_SUCCESS) {

                struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                rpc_status = serializer->serialize_get_token_size_resp(resp_buf, token_size);
            }

            attest_report_destroy(token);
        }

        call_req_set_opstatus(req, opstatus);
    }

    return rpc_status;
}

static rpc_status_t export_iak_public_key_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct attest_provider_serializer *serializer = get_attest_serializer(context, req);

    if (serializer) {

        size_t max_key_size = attest_key_mngr_max_iak_export_size();

        uint8_t *key_buffer = malloc(max_key_size);

        if (key_buffer) {

            int opstatus;
            size_t export_size;
            opstatus =
                attest_key_mngr_export_iak_public_key(key_buffer, max_key_size, &export_size);

            if (opstatus == PSA_SUCCESS) {

                struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
                rpc_status =
                    serializer->serialize_export_iak_public_key_resp(resp_buf,
                        key_buffer, export_size);
            }

            free(key_buffer);
            call_req_set_opstatus(req, opstatus);
        }
        else {
            /* Failed to allocate key buffer */
            rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
        }
    }

    return rpc_status;
}

static rpc_status_t import_iak_handler(void *context, struct call_req* req)
{
    rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct attest_provider_serializer *serializer = get_attest_serializer(context, req);

    if (serializer) {

        size_t key_data_len = attest_key_mngr_max_iak_import_size();
        uint8_t *key_buffer = malloc(key_data_len);

        if (key_buffer) {

            rpc_status =
                serializer->deserialize_import_iak_req(req_buf, key_buffer, &key_data_len);

            if (rpc_status == TS_RPC_CALL_ACCEPTED) {

                int opstatus;
                opstatus = attest_key_mngr_import_iak(key_buffer, key_data_len);
                call_req_set_opstatus(req, opstatus);
            }

            free(key_buffer);
        }
        else {

            rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
        }
    }

    return rpc_status;
}

static rpc_status_t iak_exists_handler(void *context, struct call_req* req)
{
    int opstatus = PSA_ERROR_DOES_NOT_EXIST;

    if (attest_key_mngr_iak_exists()) {

       opstatus = PSA_SUCCESS;
    }

    call_req_set_opstatus(req, opstatus);

    return TS_RPC_CALL_ACCEPTED;
}

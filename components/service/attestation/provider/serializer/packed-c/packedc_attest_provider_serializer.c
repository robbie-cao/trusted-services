/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>
#include <common/tlv/tlv.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/attestation/packed-c/get_token.h>
#include <protocols/service/attestation/packed-c/get_token_size.h>
#include "packedc_attest_provider_serializer.h"


/* Operation: get_token */
static rpc_status_t deserialize_get_token_req(const struct call_param_buf *req_buf,
    uint8_t *auth_challenge, size_t *auth_challenge_len)
{
    rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
    struct tlv_const_iterator req_iter;
    struct tlv_record decoded_record;

    tlv_const_iterator_begin(&req_iter, (uint8_t*)req_buf->data, req_buf->data_len);

    if (tlv_find_decode(&req_iter,
        TS_ATTESTATION_GET_TOKEN_IN_TAG_AUTH_CHALLENGE, &decoded_record)) {

        if (decoded_record.length <= *auth_challenge_len) {

            memcpy(auth_challenge, decoded_record.value, decoded_record.length);
            *auth_challenge_len = decoded_record.length;
            rpc_status = TS_RPC_CALL_ACCEPTED;
        }
    }

    return rpc_status;
}

static rpc_status_t serialize_get_token_resp(struct call_param_buf *resp_buf,
    const uint8_t *token, size_t token_size)
{
    rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
    struct tlv_iterator resp_iter;

    struct tlv_record token_record;
    token_record.tag = TS_ATTESTATION_GET_TOKEN_OUT_TAG_TOKEN;
    token_record.length = token_size;
    token_record.value = token;

    tlv_iterator_begin(&resp_iter, resp_buf->data, resp_buf->size);

    if (tlv_encode(&resp_iter, &token_record)) {

        resp_buf->data_len = tlv_required_space(token_size);
        rpc_status = TS_RPC_CALL_ACCEPTED;
    }

    return rpc_status;
}

/* Operation: get_token_size */
static rpc_status_t deserialize_get_token_size_req(const struct call_param_buf *req_buf,
    size_t *auth_challenge_len)
{
    rpc_status_t rpc_status = TS_RPC_ERROR_INVALID_REQ_BODY;
    struct ts_attestation_get_token_size_in recv_msg;
    size_t expected_fixed_len = sizeof(struct ts_attestation_get_token_size_in);

    if (expected_fixed_len <= req_buf->data_len) {

        memcpy(&recv_msg, req_buf->data, expected_fixed_len);
        *auth_challenge_len = recv_msg.challenge_size;
        rpc_status = TS_RPC_CALL_ACCEPTED;
    }

    return rpc_status;
}

static rpc_status_t serialize_get_token_size_resp(struct call_param_buf *resp_buf,
    size_t token_size)
{
    rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
    struct ts_attestation_get_token_size_out resp_msg;
    size_t fixed_len = sizeof(struct ts_attestation_get_token_size_out);

    resp_msg.token_size = token_size;

    if (fixed_len <= resp_buf->size) {

        memcpy(resp_buf->data, &resp_msg, fixed_len);
        resp_buf->data_len = fixed_len;
        rpc_status = TS_RPC_CALL_ACCEPTED;
    }

    return rpc_status;
}

/* Singleton method to provide access to the serializer instance */
const struct attest_provider_serializer *packedc_attest_provider_serializer_instance(void)
{
    static const struct attest_provider_serializer instance = {
        deserialize_get_token_req,
        serialize_get_token_resp,
        deserialize_get_token_size_req,
        serialize_get_token_size_resp
    };

    return &instance;
}

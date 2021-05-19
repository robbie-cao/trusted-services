/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>
#include "attest_provision_client.h"
#include <common/tlv/tlv.h>
#include <provision/attest_provision.h>
#include <protocols/service/attestation/packed-c/export_iak_public_key.h>
#include <protocols/service/attestation/packed-c/import_iak.h>
#include <protocols/service/attestation/packed-c/opcodes.h>
#include <protocols/rpc/common/packed-c/status.h>

/**
 * @brief      The singleton attest_provision_client instance
 *
 * The attest provison C API assumes a single backend service provider.  This
 * structure defines the state used by the attest_provision_client that communicates
 * with a remote provider using the provided rpc caller.
 */
static struct attest_provision_client
{
    struct rpc_caller *caller;
    int rpc_status;
} instance;


psa_status_t attest_provision_client_init(struct rpc_caller *caller)
{
	instance.caller = caller;
	instance.rpc_status = TS_RPC_CALL_ACCEPTED;

	return PSA_SUCCESS;
}

void attest_provision_client_deinit(void)
{
	instance.caller = NULL;
}

int attest_provision_client_rpc_status(void)
{
	return instance.rpc_status;
}

psa_status_t attest_provision_export_iak_public_key(
    uint8_t *data,
    size_t data_size,
    size_t *data_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;

    *data_length = 0; /* For failure case */

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(instance.caller, &req_buf, 0);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        instance.rpc_status = rpc_caller_invoke(instance.caller, call_handle,
            TS_ATTESTATION_OPCODE_EXPORT_IAK_PUBLIC_KEY, &opstatus, &resp_buf, &resp_len);

        if (instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter,
                    TS_ATTESTATION_EXPORT_IAK_PUBLIC_KEY_OUT_TAG_DATA, &decoded_record)) {

                    if (decoded_record.length <= data_size) {

                        memcpy(data, decoded_record.value, decoded_record.length);
                        *data_length = decoded_record.length;
                    }
                    else {
                        /* Provided buffer is too small */
                        psa_status = PSA_ERROR_BUFFER_TOO_SMALL;
                    }
                }
                else {
                    /* Mandatory response parameter missing */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(instance.caller, call_handle);
    }

    return psa_status;
}

psa_status_t attest_provision_import_iak(
    const uint8_t *data,
    size_t data_length)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    size_t req_len = tlv_required_space(data_length);

    struct tlv_record key_record;
    key_record.tag = TS_ATTESTATION_IMPORT_IAK_IN_TAG_DATA;
    key_record.length = data_length;
    key_record.value = data;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(instance.caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;
        struct tlv_iterator req_iter;

        tlv_iterator_begin(&req_iter, req_buf, req_len);
        tlv_encode(&req_iter, &key_record);

        instance.rpc_status = rpc_caller_invoke(instance.caller, call_handle,
            TS_ATTESTATION_OPCODE_IMPORT_IAK, &opstatus, &resp_buf, &resp_len);

        if (instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;
        }

        rpc_caller_end(instance.caller, call_handle);
    }

    return psa_status;

}

psa_status_t attest_provision_iak_exists(void)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(instance.caller, &req_buf, 0);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        instance.rpc_status = rpc_caller_invoke(instance.caller, call_handle,
            TS_ATTESTATION_OPCODE_IAK_EXISTS, &opstatus, &resp_buf, &resp_len);

        if (instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;
        }

        rpc_caller_end(instance.caller, call_handle);
    }

    return psa_status;
}

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include "psa_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/generate_random.h>
#include <common/tlv/tlv.h>

psa_status_t psa_generate_random(uint8_t *output, size_t output_size)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_generate_random_in req_msg;
    size_t req_len = sizeof(struct ts_crypto_generate_random_in);

    if (psa_crypto_client_instance.init_status != PSA_SUCCESS)
        return psa_crypto_client_instance.init_status;

    req_msg.size = output_size;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;

        memcpy(req_buf, &req_msg, req_len);

        psa_crypto_client_instance.rpc_status =
            rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
                    TS_CRYPTO_OPCODE_GENERATE_RANDOM, &opstatus, &resp_buf, &resp_len);

        if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                struct tlv_const_iterator resp_iter;
                struct tlv_record decoded_record;
                tlv_const_iterator_begin(&resp_iter, resp_buf, resp_len);

                if (tlv_find_decode(&resp_iter,
                            TS_CRYPTO_GENERATE_RANDOM_OUT_TAG_RANDOM_BYTES, &decoded_record)) {

                    if (decoded_record.length <= output_size) {

                        memcpy(output, decoded_record.value, decoded_record.length);
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

        rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
    }

    return psa_status;
}

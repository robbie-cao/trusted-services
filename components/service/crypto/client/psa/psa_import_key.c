/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include "psa_crypto_client.h"
#include "psa_crypto_client_key_attributes.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/crypto/packed-c/opcodes.h>
#include <protocols/service/crypto/packed-c/key_attributes.h>
#include <protocols/service/crypto/packed-c/import_key.h>
#include <common/tlv/tlv.h>


psa_status_t psa_import_key(const psa_key_attributes_t *attributes,
                        const uint8_t *data, size_t data_length, psa_key_id_t *id)
{
    psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
    struct ts_crypto_import_key_in req_msg;
    size_t req_fixed_len = sizeof(struct ts_crypto_import_key_in);
    size_t req_len = req_fixed_len + tlv_required_space(data_length);

    /* Set default outputs for failure case */
    *id = 0;

    if (psa_crypto_client_instance.init_status != PSA_SUCCESS)
        return psa_crypto_client_instance.init_status;

    psa_crypto_client_translate_key_attributes_to_proto(&req_msg.attributes, attributes);

    struct tlv_record key_record;
    key_record.tag = TS_CRYPTO_IMPORT_KEY_IN_TAG_DATA;
    key_record.length = data_length;
    key_record.value = data;

    rpc_call_handle call_handle;
    uint8_t *req_buf;

    call_handle = rpc_caller_begin(psa_crypto_client_instance.caller, &req_buf, req_len);

    if (call_handle) {

        uint8_t *resp_buf;
        size_t resp_len;
        int opstatus;
        struct tlv_iterator req_iter;

        memcpy(req_buf, &req_msg, req_fixed_len);

        tlv_iterator_begin(&req_iter, &req_buf[req_fixed_len], req_len - req_fixed_len);
        tlv_encode(&req_iter, &key_record);

        psa_crypto_client_instance.rpc_status =
            rpc_caller_invoke(psa_crypto_client_instance.caller, call_handle,
                        TS_CRYPTO_OPCODE_IMPORT_KEY, &opstatus, &resp_buf, &resp_len);

        if (psa_crypto_client_instance.rpc_status == TS_RPC_CALL_ACCEPTED) {

            psa_status = opstatus;

            if (psa_status == PSA_SUCCESS) {

                if (resp_len >= sizeof(struct ts_crypto_import_key_out)) {

                    struct ts_crypto_import_key_out resp_msg;
                    memcpy(&resp_msg, resp_buf, sizeof(struct ts_crypto_import_key_out));
                    *id = resp_msg.id;
                }
                else {
                    /* Failed to decode response message */
                    psa_status = PSA_ERROR_GENERIC_ERROR;
                }
            }
        }

        rpc_caller_end(psa_crypto_client_instance.caller, call_handle);
    }

    return psa_status;
}

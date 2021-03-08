/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crypto_service_context.h"
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/packed-c/packedc_crypto_provider_serializer.h>

crypto_service_context::crypto_service_context(const char *sn) :
    standalone_service_context(sn),
    m_crypto_provider(),
    m_storage_client(),
    m_null_store(),
    m_storage_service_context(NULL),
    m_storage_session_handle(NULL)
{

}

crypto_service_context::~crypto_service_context()
{

}

void crypto_service_context::do_init()
{
    struct storage_backend *storage_backend = NULL;
    struct storage_backend *null_storage_backend = null_store_init(&m_null_store);
    struct rpc_caller *storage_caller = NULL;
    int status;

    /* Locate and open RPC session with internal-trusted-storage service to provide a persistent keystore */
    m_storage_service_context = service_locator_query("sn:trustedfirmware.org:internal-trusted-storage:0", &status);

    if (m_storage_service_context) {

        m_storage_session_handle = service_context_open(m_storage_service_context, TS_RPC_ENCODING_PACKED_C, &storage_caller);

        if (m_storage_session_handle) {

            storage_backend = secure_storage_client_init(&m_storage_client, storage_caller);
        }
    }

    if (!storage_backend) {

        /* Something has gone wrong with establishing a session with the storage service endpoint */
        storage_backend = null_storage_backend;
    }

    /* Initialse the crypto service provider */
    struct rpc_interface *crypto_ep = mbed_crypto_provider_init(&m_crypto_provider, storage_backend, 0);

    mbed_crypto_provider_register_serializer(&m_crypto_provider,
                    TS_RPC_ENCODING_PROTOBUF, pb_crypto_provider_serializer_instance());

    mbed_crypto_provider_register_serializer(&m_crypto_provider,
                    TS_RPC_ENCODING_PACKED_C, packedc_crypto_provider_serializer_instance());

    standalone_service_context::set_rpc_interface(crypto_ep);
}

void crypto_service_context::do_deinit()
{
    if (m_storage_session_handle) {
        service_context_close(m_storage_service_context, m_storage_session_handle);
        m_storage_session_handle = NULL;
    }

    if (m_storage_service_context) {
        service_context_relinquish(m_storage_service_context);
        m_storage_service_context = NULL;
    }

    mbed_crypto_provider_deinit(&m_crypto_provider);
    secure_storage_client_deinit(&m_storage_client);
    null_store_deinit(&m_null_store);
}

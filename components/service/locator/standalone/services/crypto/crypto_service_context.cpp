/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crypto_service_context.h"
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/packed-c/packedc_crypto_provider_serializer.h>
#include <service/secure_storage/backend/secure_flash_store/secure_flash_store.h>

crypto_service_context::crypto_service_context(const char *sn) :
    standalone_service_context(sn),
    m_crypto_provider(),
    m_storage_provider(),
    m_storage_caller()
{

}

crypto_service_context::~crypto_service_context()
{

}

void crypto_service_context::do_init()
{
    struct storage_backend *storage_backend = sfs_init();
    struct rpc_interface *storage_ep = secure_storage_provider_init(&m_storage_provider,
                                                                storage_backend);
    struct rpc_caller *storage_caller = direct_caller_init_default(&m_storage_caller,
                                                                storage_ep);
    struct rpc_interface *crypto_ep = mbed_crypto_provider_init(&m_crypto_provider,
                                                                storage_caller, 0);

    mbed_crypto_provider_register_serializer(&m_crypto_provider,
                    TS_RPC_ENCODING_PROTOBUF, pb_crypto_provider_serializer_instance());

    mbed_crypto_provider_register_serializer(&m_crypto_provider,
                    TS_RPC_ENCODING_PACKED_C, packedc_crypto_provider_serializer_instance());

    standalone_service_context::set_rpc_interface(crypto_ep);
}

void crypto_service_context::do_deinit()
{
    mbed_crypto_provider_deinit(&m_crypto_provider);
    secure_storage_provider_deinit(&m_storage_provider);
    direct_caller_deinit(&m_storage_caller);
}

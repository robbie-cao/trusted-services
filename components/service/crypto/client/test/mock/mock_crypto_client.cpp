/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mock_crypto_client.h"
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/packed-c/packedc_crypto_provider_serializer.h>

mock_crypto_client::mock_crypto_client() :
    test_crypto_client(),
    m_crypto_provider(),
    m_storage_provider(),
    m_crypto_caller(),
    m_storage_caller()
{

}

mock_crypto_client::~mock_crypto_client()
{

}

bool mock_crypto_client::init()
{
    bool should_do = test_crypto_client::init();

    if (should_do) {

        struct rpc_interface *storage_ep = mock_store_provider_init(&m_storage_provider);
        struct rpc_caller *storage_caller = direct_caller_init_default(&m_storage_caller,
                                                                    storage_ep);

        struct rpc_interface *crypto_ep = mbed_crypto_provider_init(&m_crypto_provider,
                                                                    storage_caller);
        struct rpc_caller *crypto_caller = direct_caller_init_default(&m_crypto_caller,
                                                                    crypto_ep);

        mbed_crypto_provider_register_serializer(&m_crypto_provider,
                    TS_RPC_ENCODING_PROTOBUF, pb_crypto_provider_serializer_instance());

        mbed_crypto_provider_register_serializer(&m_crypto_provider,
                    TS_RPC_ENCODING_PACKED_C, packedc_crypto_provider_serializer_instance());

        rpc_caller_set_encoding_scheme(crypto_caller, TS_RPC_ENCODING_PROTOBUF);

        crypto_client::set_caller(crypto_caller);
    }

    return should_do;
}

bool mock_crypto_client::deinit()
{
    bool should_do = test_crypto_client::deinit();

    if (should_do) {

        mbed_crypto_provider_deinit(&m_crypto_provider);
        mock_store_provider_deinit(&m_storage_provider);

        direct_caller_deinit(&m_storage_caller);
        direct_caller_deinit(&m_crypto_caller);
    }

    return should_do;
}

/* Test Methods */
bool mock_crypto_client::keystore_reset_is_supported() const
{
    return true;
}

void mock_crypto_client::keystore_reset()
{
    mock_store_reset(&m_storage_provider);
}

bool mock_crypto_client::keystore_key_exists_is_supported() const
{
    return true;
}

bool mock_crypto_client::keystore_key_exists(uint32_t id) const
{
    return mock_store_exists(&m_storage_provider, id);
}

bool mock_crypto_client::keystore_keys_held_is_supported() const
{
    return true;
}

size_t mock_crypto_client::keystore_keys_held() const
{
    return mock_store_num_items(&m_storage_provider);
}

/* Factory for creating mock_crypto_client objects */
class mock_crypto_client_factory : public test_crypto_client::factory
{
public:
    mock_crypto_client_factory() :
        test_crypto_client::factory()
    {
        test_crypto_client::register_factory(this);
    }

    ~mock_crypto_client_factory()
    {
        test_crypto_client::deregister_factory(this);
    }

    test_crypto_client *create()
    {
        return new mock_crypto_client;
    };
};

/*
 * Static construction causes this to be registered
 * as the default factory for constructing test_crypto_client objects.
 */
static mock_crypto_client_factory default_factory;

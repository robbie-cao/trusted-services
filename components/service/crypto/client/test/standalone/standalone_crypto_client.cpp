/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "standalone_crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/psa/packed-c/status.h>
#include <service/crypto/provider/serializer/protobuf/pb_crypto_provider_serializer.h>
#include <service/crypto/provider/serializer/packed-c/packedc_crypto_provider_serializer.h>

standalone_crypto_client::standalone_crypto_client() :
    test_crypto_client(),
    m_crypto_provider(),
    m_storage_provider(),
    m_crypto_caller(),
    m_storage_caller(),
    m_dummy_storage_caller()
{

}

standalone_crypto_client::~standalone_crypto_client()
{

}

bool standalone_crypto_client::init()
{
    bool should_do = test_crypto_client::init();

    if (should_do) {

        struct rpc_caller *storage_caller;

        if (!is_fault_injected(FAILED_TO_DISCOVER_SECURE_STORAGE)) {

            /* Establish rpc session with storage provider */
            struct rpc_interface *storage_ep = sfs_provider_init(&m_storage_provider);
            storage_caller = direct_caller_init_default(&m_storage_caller, storage_ep);
        }
        else {

            /*
             * Missing storage service fault injected.  To allow a somewhat viable
             * crypto service to be started, use a dummy _caller that will safely
             * terminate storage calls with an appropriate error.
             */
            storage_caller = dummy_caller_init(&m_dummy_storage_caller,
                        TS_RPC_CALL_ACCEPTED, PSA_ERROR_STORAGE_FAILURE);
        }

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

bool standalone_crypto_client::deinit()
{
    bool should_do = test_crypto_client::deinit();

    if (should_do) {

        mbed_crypto_provider_deinit(&m_crypto_provider);

        direct_caller_deinit(&m_storage_caller);
        direct_caller_deinit(&m_crypto_caller);
    }

    return should_do;
}

/* Fault injection */
bool standalone_crypto_client::is_fault_supported(enum fault_code code) const
{
    return (code == test_crypto_client::FAILED_TO_DISCOVER_SECURE_STORAGE);
}

/* Test Methods */
bool standalone_crypto_client::keystore_reset_is_supported() const
{
    return test_crypto_client::keystore_reset_is_supported();
}

void standalone_crypto_client::keystore_reset()
{
    test_crypto_client::keystore_reset();
}

bool standalone_crypto_client::keystore_key_exists_is_supported() const
{
    return test_crypto_client::keystore_key_exists_is_supported() ;
}

bool standalone_crypto_client::keystore_key_exists(uint32_t id) const
{
    return test_crypto_client::keystore_key_exists(id);
}

bool standalone_crypto_client::keystore_keys_held_is_supported() const
{
    return test_crypto_client::keystore_keys_held_is_supported();
}

size_t standalone_crypto_client::keystore_keys_held() const
{
    return test_crypto_client::keystore_keys_held();
}

/* Factory for creating standalone_crypto_client objects */
class standalone_crypto_client_factory : public test_crypto_client::factory
{
public:
    standalone_crypto_client_factory() :
        test_crypto_client::factory()
    {
        test_crypto_client::register_factory(this);
    }

    ~standalone_crypto_client_factory()
    {
        test_crypto_client::deregister_factory(this);
    }

    test_crypto_client *create()
    {
        return new standalone_crypto_client;
    };
};

/*
 * Static construction causes this to be registered
 * as the default factory for constructing test_crypto_client objects.
 */
static standalone_crypto_client_factory default_factory;

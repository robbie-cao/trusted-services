/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "standalone_crypto_client.h"
#include <service/crypto/factory/crypto_provider_factory.h>
#include <service/crypto/backend/mbedcrypto/mbedcrypto_backend.h>
#include <service/secure_storage/backend/secure_flash_store/secure_flash_store.h>
#include <service/secure_storage/backend/secure_flash_store/flash/ram/sfs_flash_ram.h>

standalone_crypto_client::standalone_crypto_client() :
    test_crypto_client(),
    m_crypto_provider(NULL),
    m_storage_provider(),
    m_storage_client(),
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
            struct storage_backend *storage_backend = sfs_init(sfs_flash_ram_instance());
            struct rpc_interface *storage_ep = secure_storage_provider_init(&m_storage_provider,
                                                                storage_backend);
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

        struct rpc_interface *crypto_iface = NULL;
        struct storage_backend *client_storage_backend =
            secure_storage_client_init(&m_storage_client, storage_caller);

        if (mbedcrypto_backend_init(client_storage_backend, 0) == PSA_SUCCESS) {

            m_crypto_provider = crypto_provider_factory_create();
            crypto_iface = service_provider_get_rpc_interface(&m_crypto_provider->base_provider);
        }

        struct rpc_caller *crypto_caller = direct_caller_init_default(&m_crypto_caller, crypto_iface);
        rpc_caller_set_encoding_scheme(crypto_caller, TS_RPC_ENCODING_PROTOBUF);

        crypto_client::set_caller(crypto_caller);
    }

    return should_do;
}

bool standalone_crypto_client::deinit()
{
    bool should_do = test_crypto_client::deinit();

    if (should_do) {

        crypto_provider_factory_destroy(m_crypto_provider);
        secure_storage_provider_deinit(&m_storage_provider);
        secure_storage_client_deinit(&m_storage_client);

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

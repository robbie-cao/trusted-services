/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_CRYPTO_CLIENT_H
#define STANDALONE_CRYPTO_CLIENT_H

#include <service/crypto/client/test/test_crypto_client.h>
#include <rpc/direct/direct_caller.h>
#include <rpc/dummy/dummy_caller.h>
#include <service/crypto/provider/crypto_provider.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>

/*
 * A specialization of the crypto_client class that extends it to add crypto
 * and storage providers to offer a viable crypto service from a single object.
 * This is only used for test purposes and should not be used for production
 * deployments.  Provides methods used for inspecting service state that
 * support test.
 */
class standalone_crypto_client : public test_crypto_client
{
public:
    standalone_crypto_client();
    virtual ~standalone_crypto_client();

    bool init();
    bool deinit();

    /* Test support methods */
    bool keystore_reset_is_supported() const;
    void keystore_reset();

    bool keystore_key_exists_is_supported() const;
    bool keystore_key_exists(uint32_t id) const;

    bool keystore_keys_held_is_supported() const;
    size_t keystore_keys_held() const;

private:
    bool is_fault_supported(enum fault_code code) const;

    struct crypto_provider *m_crypto_provider;
    struct secure_storage_provider m_storage_provider;
    struct secure_storage_client m_storage_client;
    struct rpc_caller_interface m_crypto_caller;
    struct rpc_caller_interface m_storage_caller;
    struct rpc_caller_session m_crypto_session;
    struct rpc_caller_session m_storage_session;
};

#endif /* STANDALONE_CRYPTO_CLIENT_H */

/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_CRYPTO_CLIENT_H
#define STANDALONE_CRYPTO_CLIENT_H

#include <service/crypto/client/test/test_crypto_client.h>
#include <rpc/direct/direct_caller.h>
#include <rpc/dummy/dummy_caller.h>
#include <service/crypto/provider/mbedcrypto/crypto_provider.h>
#include <service/secure_storage/provider/secure_flash_store/sfs_provider.h>

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

    struct mbed_crypto_provider m_crypto_provider;
    struct sfs_provider m_storage_provider;
    struct direct_caller m_crypto_caller;
    struct direct_caller m_storage_caller;
    struct dummy_caller m_dummy_storage_caller;
};

#endif /* STANDALONE_CRYPTO_CLIENT_H */

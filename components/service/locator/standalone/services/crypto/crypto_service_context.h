/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_CRYPTO_SERVICE_CONTEXT_H
#define STANDALONE_CRYPTO_SERVICE_CONTEXT_H

#include <service/locator/standalone/standalone_service_context.h>
#include <rpc/direct/direct_caller.h>
#include <service/crypto/provider/mbedcrypto/crypto_provider.h>
#include <service/secure_storage/provider/secure_flash_store/sfs_provider.h>

class crypto_service_context : public standalone_service_context
{
public:
    crypto_service_context(const char *sn);
    virtual ~crypto_service_context();

private:

    void do_init();
    void do_deinit();

    struct mbed_crypto_provider m_crypto_provider;
    struct sfs_provider m_storage_provider;
    struct direct_caller m_storage_caller;
};

#endif /* STANDALONE_CRYPTO_SERVICE_CONTEXT_H */

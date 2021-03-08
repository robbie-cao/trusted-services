/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_CRYPTO_SERVICE_CONTEXT_H
#define STANDALONE_CRYPTO_SERVICE_CONTEXT_H

#include <service/locator/standalone/standalone_service_context.h>
#include <rpc/direct/direct_caller.h>
#include <service/crypto/provider/mbedcrypto/crypto_provider.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/backend/null_store/null_store.h>

class crypto_service_context : public standalone_service_context
{
public:
    crypto_service_context(const char *sn);
    virtual ~crypto_service_context();

private:

    void do_init();
    void do_deinit();

    struct mbed_crypto_provider m_crypto_provider;
    struct secure_storage_client m_storage_client;
    struct null_store m_null_store;
    struct service_context *m_storage_service_context;
    rpc_session_handle m_storage_session_handle;
};

#endif /* STANDALONE_CRYPTO_SERVICE_CONTEXT_H */

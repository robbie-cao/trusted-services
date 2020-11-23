/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crypto_service_context.h"

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
    struct call_ep *storage_ep = sfs_provider_init(&m_storage_provider);
    struct rpc_caller *storage_caller = direct_caller_init_default(&m_storage_caller, storage_ep);
    struct call_ep *crypto_ep = mbed_crypto_provider_init(&m_crypto_provider, storage_caller);

    standalone_service_context::set_call_ep(crypto_ep);
}

void crypto_service_context::do_deinit()
{
    mbed_crypto_provider_deinit(&m_crypto_provider);
    direct_caller_deinit(&m_storage_caller);
}

/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>

crypto_client::crypto_client() :
    m_client()
{
    service_client_init(&m_client, NULL);
}

crypto_client::crypto_client(struct rpc_caller *caller) :
    m_client()
{
    service_client_init(&m_client, caller);
}

crypto_client::~crypto_client()
{
    service_client_deinit(&m_client);
}

void crypto_client::set_caller(struct rpc_caller *caller)
{
    m_client.caller = caller;
}

int crypto_client::err_rpc_status() const
{
    return m_client.rpc_status;
}

/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crypto_client.h"
#include <protocols/rpc/common/packed-c/status.h>

crypto_client::crypto_client() :
    m_caller(NULL),
    m_err_rpc_status(TS_RPC_CALL_ACCEPTED)
{

}

crypto_client::crypto_client(struct rpc_caller *caller) :
    m_caller(caller),
    m_err_rpc_status(TS_RPC_CALL_ACCEPTED)
{

}

crypto_client::~crypto_client()
{

}

void crypto_client::set_caller(struct rpc_caller *caller)
{
    m_caller = caller;
}

int crypto_client::err_rpc_status() const
{
    return m_err_rpc_status;
}

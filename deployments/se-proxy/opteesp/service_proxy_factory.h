/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SERVICE_PROXY_FACTORY_H
#define SERVICE_PROXY_FACTORY_H

#include <rpc/common/endpoint/rpc_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rpc_interface *attest_proxy_create(void);
struct rpc_interface *crypto_proxy_create(void);
struct rpc_interface *ps_proxy_create(void);
struct rpc_interface *its_proxy_create(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_PROXY_FACTORY_H */

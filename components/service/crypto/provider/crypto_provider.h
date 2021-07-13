/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CRYPTO_PROVIDER_H
#define CRYPTO_PROVIDER_H

#include <rpc/common/endpoint/rpc_interface.h>
#include <service/common/provider/service_provider.h>
#include <service/crypto/provider/serializer/crypto_provider_serializer.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include "crypto_context_pool.h"

#ifdef __cplusplus
extern "C" {
#endif

struct crypto_provider
{
    struct service_provider base_provider;
    struct crypto_context_pool context_pool;
    const struct crypto_provider_serializer *serializers[TS_RPC_ENCODING_LIMIT];
};

/*
 * Initializes an instance of the crypto service provider.  A suitable
 * backend that realizes the PSA Crypto API should have been initialized
 * prior to initializing the crypto provider.
 */
struct rpc_interface *crypto_provider_init(struct crypto_provider *context);

/*
 * When operation of the provider is no longer required, this function
 * frees any resource used by the previously initialized provider instance.
 */
void crypto_provider_deinit(struct crypto_provider *context);

/*
 * Register a serializer for supportng a particular parameter encoding.  At
 * least one serializer must be registered but additional ones may be registered
 * to allow alternative parameter serialization schemes to be used to allow
 * for compatibility with different types of client.
 */
void crypto_provider_register_serializer(struct crypto_provider *context,
                    unsigned int encoding, const struct crypto_provider_serializer *serializer);

/*
 * Extend the core set of operations provided by the crypto provider.
 */
void crypto_provider_extend(struct crypto_provider *context,
                    struct service_provider *sub_provider);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CRYPTO_PROVIDER_H */

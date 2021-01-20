/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MBED_CRYPTO_PROVIDER_H
#define MBED_CRYPTO_PROVIDER_H

#include <rpc/common/endpoint/rpc_interface.h>
#include <rpc_caller.h>
#include <service/common/provider/service_provider.h>
#include <service/crypto/provider/serializer/crypto_provider_serializer.h>
#include <protocols/rpc/common/packed-c/encoding.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mbed_crypto_provider
{
    struct service_provider base_provider;
    const struct crypto_provider_serializer *serializers[TS_RPC_ENCODING_LIMIT];
};

/*
 * Initializes an instance of the crypto service provider that uses the
 * Mbed Crypto library to implement crypto operations.  Secure storage
 * for persistent keys needs to be provided by a suitable storage
 * provider, accessed using the secure storage service access protocol
 * using the provided rpc_caller.  Any rpc endpoint discovery and
 * session establishment should have been performed prior to initializing
 * the mbed_crypto_provider.  On successfully initializing the provider,
 * a pointer to the rpc_interface for the service is returned.
 */
struct rpc_interface *mbed_crypto_provider_init(struct mbed_crypto_provider *context,
                                        struct rpc_caller *storage_provider,
                                        void *entropy_adapter_config);

/*
 * When operation of the provider is no longer required, this function
 * frees any resource used by the previously initialized provider instance.
 */
void mbed_crypto_provider_deinit(struct mbed_crypto_provider *context);

/*
 * Register a serializer for supportng a particular parameter encoding.  At
 * least one serializer must be registered but additional ones may be registered
 * to allow alternative parameter serialization schemes to be used to allow
 * for compatibility with different types of client.
 */
void mbed_crypto_provider_register_serializer(struct mbed_crypto_provider *context,
                    unsigned int encoding, const struct crypto_provider_serializer *serializer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MBED_CRYPTO_PROVIDER_H */

/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MBED_CRYPTO_PROVIDER_H
#define MBED_CRYPTO_PROVIDER_H

#include <rpc/common/endpoint/call_ep.h>
#include <rpc_caller.h>
#include <service/common/provider/service_provider.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mbed_crypto_provider
{
    struct service_provider base_provider;
};

/*
 * Initializes an instance of the crypto service provider that uses the
 * Mbed Crypto library to implement crypto operations.  Secure storage
 * for persistent keys needs to be provided by a suitable storage
 * provider, accessed using the secure storage service access protocol
 * using the provided rpc_caller.  Any rpc endpoint discovery and
 * session establishment should have been performed prior to initializing
 * the mbed_crypto_provider.  On successfully initializing the provider,
 * a pointer to the call_ep for the service is returned.
 */
struct call_ep *mbed_crypto_provider_init(struct mbed_crypto_provider *context,
                                        struct rpc_caller *storage_provider);

/*
 * When operation of the provider is no longer required, this function
 * frees any resource used by the previously initialized provider instance.
 */
void mbed_crypto_provider_deinit(struct mbed_crypto_provider *context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MBED_CRYPTO_PROVIDER_H */

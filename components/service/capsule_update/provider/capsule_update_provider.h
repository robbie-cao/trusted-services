/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CAPSULE_UPDATE_PROVIDER_H
#define CAPSULE_UPDATE_PROVIDER_H

#include <rpc/common/endpoint/rpc_interface.h>
#include <service/common/provider/service_provider.h>
#include <service/common/client/service_client.h>
#include <service/capsule_update/backend/capsule_update_backend.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The capsule_update_provider is a service provider that accepts update capsule
 * requests and delegates them to a suitable backend that applies the update.
 */
struct capsule_update_provider
{
	struct service_provider base_provider;
	struct service_client client;
};

/**
 * \brief Initialize an instance of the capsule update service provider
 *
 * @param[in] context The instance to initialize
 *
 * \return An rpc_interface or NULL on failure
 */
struct rpc_interface *capsule_update_provider_init(
	struct capsule_update_provider *context);

/**
 * \brief Cleans up when the instance is no longer needed
 *
 * \param[in] context   The instance to de-initialize
 */
void capsule_update_provider_deinit(
	struct capsule_update_provider *context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CAPSULE_UPDATE_PROVIDER_H */

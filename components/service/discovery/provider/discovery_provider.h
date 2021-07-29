/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISCOVERY_PROVIDER_H
#define DISCOVERY_PROVIDER_H

#include <stdint.h>
#include <service/common/provider/service_provider.h>
#include <service/discovery/provider/serializer/discovery_provider_serializer.h>
#include <service/discovery/provider/discovery_info.h>
#include <protocols/rpc/common/packed-c/encoding.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Instance data for a discover_provider object.
 */
struct discovery_provider
{
	struct service_provider base_provider;
	const struct discovery_provider_serializer *serializers[TS_RPC_ENCODING_LIMIT];

	struct discovery_info info;
};

/**
 * Initializes an instance of the discovery service provider.
 */
void discovery_provider_init(
	struct discovery_provider *context);

/**
 * When operation of the provider is no longer required, this function
 * frees any resource used by the previously initialized provider instance.
 */
void discovery_provider_deinit(
	struct discovery_provider *context);

/**
 * Register a serializer for supportng a particular parameter encoding.
 */
void discovery_provider_register_serializer(
	struct discovery_provider *context,
	unsigned int encoding,
	const struct discovery_provider_serializer *serializer);

/**
 * Register a supported protocol encoding for the service provider that
 * this discovery provider represents.
 */
void discovery_provider_register_supported_encoding(
	struct discovery_provider *context,
	unsigned int encoding);

/**
 * Sets deployment specific information that is adverstised by the
 * discovery provider.
 */
void discovery_provider_set_deployment_info(
	struct discovery_provider *context,
	const struct discovery_deployment_info *deployment_info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISCOVERY_PROVIDER_H */

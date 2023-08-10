/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2023, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <psa/sid.h>
#include <rpc/common/endpoint/rpc_interface.h>
#include <service/attestation/provider/attest_provider.h>
#include <service/attestation/provider/serializer/packed-c/packedc_attest_provider_serializer.h>
#include <service/crypto/factory/crypto_provider_factory.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <trace.h>

struct rpc_interface *attest_proxy_create(void)
{
	struct rpc_interface *attest_iface = NULL;

	return attest_iface;
}

struct rpc_interface *crypto_proxy_create(void)
{
	struct rpc_interface *crypto_iface = NULL;

	return crypto_iface;
}

struct rpc_interface *ps_proxy_create(void)
{
	struct rpc_interface *rpc_interface = NULL;

	return rpc_interface;
}

struct rpc_interface *its_proxy_create(void)
{
	struct rpc_interface *rpc_interface = NULL;

	return rpc_interface;
}

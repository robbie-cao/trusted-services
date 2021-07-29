/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISCOVERY_INFO_H
#define DISCOVERY_INFO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Information about the service deployment.
 */
struct discovery_deployment_info
{
	/**
	 * The RPC interface id that should be used for directing call
	 * requests to this service provider instance.
	 */
	uint16_t interface_id;

	/**
	 * The instance number assigned to this service provider instance.
	 * This can be used by a client for identifying a particular instance
	 * of a service provider for cases where multple instances of the same
	 * type of service provider are deployed.  The instance may be reflected
	 * to clients using a standard convention such as a device numder
	 * e.g. /dev/ps0 on Linux.
	 */
	uint16_t instance;

	/**
	 * When the discovery provider and associated service provider are
	 * co-located (e.g. running in the same SP or TA), the max_payload
	 * value reported by the discovery provider is determined from the
	 * response buffer size associated with an incoming call_req object.
	 * However in cases where call requests are forwarded, say to a
	 * secure enclave, a different max_payload value may apply.  This
	 * value allows for a deployment specific override that will
	 * be reported instead.  Should be set to zero if no override
	 * applies.
	 */
	size_t max_payload_override;
};

/**
 * Aggregate of all discovery info
 */
struct discovery_info
{
	struct discovery_deployment_info deployment;

	uint32_t supported_encodings;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISCOVERY_INFO_H */

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISCOVERY_INFO_H
#define DISCOVERY_INFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Information about the service deployment.
 */
struct discovery_deployment_info
{
	uint16_t interface_id;
	uint16_t instance;
	uint32_t max_req_size;
};

/**
 * Information about service identity
 */
struct discovery_identity_info
{
	const char *name_authority;
	const char *service_name;
};

/**
 * Aggregate of all discovery info
 */
struct discovery_info
{
	struct discovery_deployment_info deployment;
	struct discovery_identity_info identity;

	uint32_t supported_encodings;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISCOVERY_INFO_H */

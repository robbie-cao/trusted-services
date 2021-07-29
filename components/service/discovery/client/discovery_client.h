/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISCOVERY_CLIENT_H
#define DISCOVERY_CLIENT_H

#include <psa/error.h>
#include <service/common/client/service_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get service info
 *
 * Discover service information from the service provider
 * instance associated with the provided service_client
 * object. If information is discovered, the service_client
 * object is updated with the discovered information.
 *
 * @param[in]  service_client 	An initialized service client
 *
 * @return     Success if information discovered
 */
psa_status_t discovery_client_get_service_info(
	struct service_client *service_client);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISCOVERY_CLIENT_H */

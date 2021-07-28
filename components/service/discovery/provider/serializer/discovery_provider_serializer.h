/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISCOVERY_PROVIDER_SERIALIZER_H
#define DISCOVERY_PROVIDER_SERIALIZER_H

#include <service/discovery/provider/discovery_info.h>
#include <rpc/common/endpoint/rpc_interface.h>

/* Provides a common interface for parameter serialization operations
 * for the discovery service provider.  Allows alternative serialization
 * protocols to be used without hard-wiring a particular protocol
 * into the service provider code.  A concrete serializer must
 * implement this interface.
 */
struct discovery_provider_serializer {

	/* Operation: get_service_info */
	rpc_status_t (*serialize_get_service_info_resp)(struct call_param_buf *resp_buf,
		const struct discovery_info *info);
};

#endif /* DISCOVERY_PROVIDER_SERIALIZER_H */

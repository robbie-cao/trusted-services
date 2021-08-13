/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include "spffa_location_strategy.h"
#include "spffa_service_context.h"
#include <common/uuid/uuid.h>
#include <service/locator/service_name.h>
#include <rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <trace.h>

static struct service_context *query(const char *sn, int *status);
static bool discover_partition(const struct uuid_octets *uuid, uint16_t *partition_id);

const struct service_location_strategy *spffa_location_strategy(void)
{
	static const struct service_location_strategy strategy = { query };
	return &strategy;
}

/**
 * This service location strategy is intended for locating other service
 * endpoints reachable via FFA from within a client secure partition where
 * associated service endpoints are explicitly defined by configuration data.
 * The service to locate is specified by a service name that consists of a
 * UUID and an optional instance number.  If pesent, the instance number
 * is treated as the destination RPC interface id.  If not specified,
 * an interface id of zero is assumed.
 */
static struct service_context *query(const char *sn, int *status)
{
	struct service_context *result = NULL;

	/* This strategy only locates endpoints reachable via FFA */
	if (sn_check_authority(sn, "ffa")) {

		struct uuid_canonical uuid;

		if (sn_read_service(sn, uuid.characters, UUID_CANONICAL_FORM_LEN + 1) &&
			uuid_is_valid(uuid.characters)) {

			uint16_t partition_id;
			struct uuid_octets uuid_octets;

			uuid_parse_to_octets(uuid.characters, uuid_octets.octets, UUID_OCTETS_LEN);

			if (discover_partition(&uuid_octets, &partition_id)) {

				unsigned int iface_id =
					sn_get_service_instance(sn);

				struct spffa_service_context *new_context =
					spffa_service_context_create(partition_id, iface_id);

				if (new_context) {

					result = &new_context->service_context;
				}
				else {

					EMSG("locate query: context create failed");
				}
			}
			else {

				EMSG("locate query: partition not discovered");
			}
		}
		else {

			EMSG("locate query: invalid uuid");
		}
	}

	return result;
}

static bool discover_partition(const struct uuid_octets *uuid, uint16_t *partition_id)
{
	bool discovered = false;
	struct ffarpc_caller ffarpc_caller;
	uint16_t discovered_partitions[1];

	ffarpc_caller_init(&ffarpc_caller);

	if (ffarpc_caller_discover(uuid->octets,
			discovered_partitions, sizeof(discovered_partitions)/sizeof(uint16_t))) {

		*partition_id = discovered_partitions[0];
		discovered = true;
	}

	ffarpc_caller_deinit(&ffarpc_caller);

	return discovered;
}

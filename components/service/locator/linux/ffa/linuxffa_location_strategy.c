/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "linuxffa_location_strategy.h"
#include "linuxffa_service_context.h"
#include <common/uuid/uuid.h>
#include <service/locator/service_name.h>
#include <rpc/ffarpc/caller/linux/ffarpc_caller.h>
#include <deployments/se-proxy/se_proxy_interfaces.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * There is the potential for there to be alternative deployment possibilities
 * for a service instance.  This will depend on deployment decisions such as
 * running one service instance per SP, or multiple services per SP.  The FFA
 * location strategy accommodates this by allowing for more than one suggestion
 * for a service to partition mapping.
 */
#define MAX_PARTITION_SUGGESTIONS           (4)

/*
 * The maximum number of partition instances, identified by a particular UUID,
 * that may be discovered.
 */
#define MAX_PARTITION_INSTANCES             (4)

/*
 * Structure to specify the location of a service endpoint reachable via FFA.
 */
struct ffa_service_location
{
	struct uuid_canonical uuid;
	uint16_t iface_id;
};


static struct service_context *query(const char *sn,
	int *status);
static size_t suggest_tf_org_partition_uuids(const char *sn,
	struct ffa_service_location *candidates, size_t candidate_limit);
static size_t use_ffa_partition_uuid(const char *sn,
	struct ffa_service_location *candidates, size_t candidate_limit);
static bool discover_partition(const char *sn,
	struct uuid_canonical *uuid, const char *dev_path, uint16_t *partition_id);

const struct service_location_strategy *linuxffa_location_strategy(void)
{
	static const struct service_location_strategy strategy = { query };
	return &strategy;
}

static struct service_context *query(const char *sn, int *status)
{
	struct service_context *result = NULL;
	struct ffa_service_location candidate_locations[MAX_PARTITION_SUGGESTIONS];
	size_t num_suggestons = 0;
	size_t suggeston_index;

	/* Determine one or more candidate partition UUIDs from the specified service name. */
	if (sn_check_authority(sn, "trustedfirmware.org")) {
		num_suggestons =
			suggest_tf_org_partition_uuids(sn, candidate_locations, MAX_PARTITION_SUGGESTIONS);
	}
	else if (sn_check_authority(sn, "ffa")) {
		num_suggestons =
			use_ffa_partition_uuid(sn, candidate_locations, MAX_PARTITION_SUGGESTIONS);
	}

	/* Attempt to discover suitable partitions */
	for (suggeston_index = 0; suggeston_index < num_suggestons; ++suggeston_index) {

		uint16_t partition_id;
		const char *dev_path = "/sys/kernel/debug/arm_ffa_user";

		if (discover_partition(sn, &candidate_locations[suggeston_index].uuid,
			dev_path, &partition_id)) {

			struct linuxffa_service_context *new_context =
				linuxffa_service_context_create(dev_path,
					partition_id, candidate_locations[suggeston_index].iface_id);

			if (new_context) result = &new_context->service_context;
			break;
		}
	}

	return result;
}

/*
 * Returns a list of partition UUIDs and interface IDs to identify partitions that
 * could potentially host the requested service.  This mapping is based trustedfirmware.org
 * ffa partition UUIDs. There may be multiple UUIDs because of different depeloyment decisions
 * such as dedicated SP, SP hosting multple services.
 */
static size_t suggest_tf_org_partition_uuids(const char *sn,
	struct ffa_service_location *candidates, size_t candidate_limit)
{
	const struct service_to_uuid
	{
		const char *service;
		const char *uuid;
		uint16_t iface_id;
	}
	partition_lookup[] =
	{
		/* Services in dedicated SPs */
		{"crypto",                      "d9df52d5-16a2-4bb2-9aa4-d26d3b84e8c0",	0},
		{"internal-trusted-storage",    "dc1eef48-b17a-4ccf-ac8b-dfcff7711b14",	0},
		{"protected-storage",           "751bf801-3dde-4768-a514-0f10aeed1790",	0},
		{"test-runner",                 "33c75baf-ac6a-4fe4-8ac7-e9909bee2d17",	0},
		{"attestation",                 "a1baf155-8876-4695-8f7c-54955e8db974",	0},

		/* Secure Enclave proxy accessed services */
		{"crypto",                      "46bb39d1-b4d9-45b5-88ff-040027dab249",	SE_PROXY_INTERFACE_ID_CRYPTO},
		{"internal-trusted-storage",    "46bb39d1-b4d9-45b5-88ff-040027dab249",	SE_PROXY_INTERFACE_ID_ITS},
		{"protected-storage",           "46bb39d1-b4d9-45b5-88ff-040027dab249",	SE_PROXY_INTERFACE_ID_PS},
		{"attestation",                 "46bb39d1-b4d9-45b5-88ff-040027dab249",	SE_PROXY_INTERFACE_ID_ATTEST},
		{NULL,                          NULL,									0}
	};

	const struct service_to_uuid *entry = &partition_lookup[0];
	size_t num_suggestions = 0;

	while (entry->service && (num_suggestions < candidate_limit)) {

		if (sn_check_service(sn, entry->service)) {

			memcpy(candidates[num_suggestions].uuid.characters, entry->uuid,
				UUID_CANONICAL_FORM_LEN + 1);

			candidates[num_suggestions].iface_id = entry->iface_id;

			++num_suggestions;
		}

		++entry;
	}

	return num_suggestions;
}

/*
 * When an ffa service name where the service field is an explicit UUID is used, the UUID
 * is used directly for partition discovery.
 */
static size_t use_ffa_partition_uuid(const char *sn,
	struct ffa_service_location *candidates, size_t candidate_limit)
{
	size_t num_suggestions = 0;

	if ((num_suggestions < candidate_limit) &&
		(sn_read_service(sn, candidates[num_suggestions].uuid.characters,
			UUID_CANONICAL_FORM_LEN + 1) == UUID_CANONICAL_FORM_LEN)) {

		candidates[num_suggestions].iface_id = 0;

		++num_suggestions;
	}

	return num_suggestions;
}

/*
 * Attempt to discover the partition that hosts the requested service instance.
 */
static bool discover_partition(const char *sn,
	struct uuid_canonical *uuid, const char *dev_path, uint16_t *partition_id)
{
	bool discovered = false;

	if (uuid_is_valid(uuid->characters) == UUID_CANONICAL_FORM_LEN) {

		struct ffarpc_caller ffarpc_caller;
		unsigned int required_instance = sn_get_service_instance(sn);

		if (!ffarpc_caller_check_version())
				return false;

		ffarpc_caller_init(&ffarpc_caller, dev_path);

		uint16_t discovered_partitions[MAX_PARTITION_INSTANCES];
		size_t discovered_count;

		discovered_count = ffarpc_caller_discover(&ffarpc_caller, uuid,
										discovered_partitions, MAX_PARTITION_INSTANCES);

		if ((discovered_count > 0) && (required_instance < discovered_count)) {

			*partition_id = discovered_partitions[required_instance];
			discovered = true;
		}

		ffarpc_caller_deinit(&ffarpc_caller);
	}

	return discovered;
}

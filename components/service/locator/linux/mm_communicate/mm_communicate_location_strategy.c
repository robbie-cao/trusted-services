/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "mm_communicate_location_strategy.h"
#include "mm_communicate_service_context.h"
#include <common/uuid/uuid.h>
#include <service/locator/service_name.h>
#include <rpc/mm_communicate/caller/linux/mm_communicate_caller.h>
#include <protocols/service/smm_variable/smm_variable_proto.h>

#define MAX_PARTITION_INSTANCES		(1)

/* Structure to define the location of an smm service */
struct smm_service_location
{
	struct uuid_canonical uuid;
	uint16_t partition_id;
	EFI_GUID svc_guid;
};

/* Structure to map a service name to FFA and MM Communicate labels */
struct smm_service_label
{
	const char *service;
	const char *uuid;
	EFI_GUID svc_guid;
};

bool find_candidate_location(const char *sn, struct smm_service_location *location)
{
	static const struct smm_service_label service_lookup[] =
	{
		{
			.service = "smm-variable",
			.uuid = "ed32d533-99e6-4209-9cc0-2d72cdd998a7",
			.svc_guid = SMM_VARIABLE_GUID
		},
		{
			/* Terminator */
			.service = NULL
		}
	};

	bool found = false;
	const struct smm_service_label *entry = &service_lookup[0];

	while (entry->service) {

		if (sn_check_service(sn, entry->service)) {

			/* Found a match */
			memcpy(location->uuid.characters,
				entry->uuid,
				UUID_CANONICAL_FORM_LEN + 1);

			location->svc_guid = entry->svc_guid;

			found = true;
			break;
		}

		++entry;
	}

	return found;
}

bool discover_partition(const char *dev_path, struct smm_service_location *location)
{
	bool discovered = false;

	if (uuid_is_valid(location->uuid.characters) == UUID_CANONICAL_FORM_LEN) {

		struct mm_communicate_caller mm_communicate_caller;

		mm_communicate_caller_init(&mm_communicate_caller, dev_path);

		uint16_t discovered_partitions[MAX_PARTITION_INSTANCES];
		size_t discovered_count;

		discovered_count = mm_communicate_caller_discover(
			&mm_communicate_caller,
			&location->uuid,
			discovered_partitions,
			MAX_PARTITION_INSTANCES);

		if (discovered_count > 0) {

			location->partition_id = discovered_partitions[0];
			discovered = true;
		}

		mm_communicate_caller_deinit(&mm_communicate_caller);
	}

	return discovered;
}

static struct service_context *query(const char *sn, int *status)
{
	struct service_context *result = NULL;

	if (!sn_check_authority(sn, "trustedfirmware.org")) return NULL;

	struct smm_service_location location;
	const char *dev_path = "/sys/kernel/debug/arm_ffa_user";

	if (find_candidate_location(sn, &location) &&
		discover_partition(dev_path, &location)) {

		struct mm_communicate_service_context *new_context =
			mm_communicate_service_context_create(
				dev_path,
				location.partition_id,
				&location.svc_guid);

		if (new_context) result = &new_context->service_context;
	}

	return result;
}

const struct service_location_strategy *mm_communicate_location_strategy(void)
{
	static const struct service_location_strategy strategy = { query };
	return &strategy;
}

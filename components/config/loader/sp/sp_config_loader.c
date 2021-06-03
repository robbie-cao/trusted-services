// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <string.h>
#include <config/interface/config_store.h>
#include <config/interface/config_blob.h>
#include <platform/interface/device_region.h>
#include "sp_config_loader.h"


struct sp_param_device_region
{
	char name[16];
	uintptr_t location;
	size_t size;
};

static void load_device_regions(const struct ffa_name_value_pair *value_pair);
static void load_memory_regions(const struct ffa_name_value_pair *value_pair);
static void load_blob(const struct ffa_name_value_pair *value_pair);

/**
 * Loads externally provided configuration data passed into the SP via
 * FFA initialisation parameters.  Data can originate from
 * the SP manifest, an external device tree or a dynamic configuration
 * mechanism such as a handover block (HOB).
 */
void sp_config_load(struct ffa_init_info *init_info)
{
	/* Load deployment specific configuration */
	for (size_t param_index = 0; param_index < init_info->count; param_index++) {

   		if (!strcmp((const char *)init_info->nvp[param_index].name,"DEVICE_REGIONS")) {

			load_device_regions(&init_info->nvp[param_index]);
		}
		else if (!strcmp((const char *)init_info->nvp[param_index].name,"MEMORY_REGIONS")) {

			load_memory_regions(&init_info->nvp[param_index]);
		}
		else {

			load_blob(&init_info->nvp[param_index]);
		}
	}
}

static void load_device_regions(const struct ffa_name_value_pair *value_pair)
{
	struct sp_param_device_region *d = (struct sp_param_device_region *)value_pair->value;

	/* Iterate over the device regions */
	while ((uintptr_t)d < (value_pair->value + value_pair->size)) {

		struct device_region device_region;

		strcpy(device_region.dev_class, d->name);
		device_region.dev_instance = 0;
		device_region.base_addr = d->location;
		device_region.io_region_size = d->size;

		config_store_add(CONFIG_CLASSIFIER_DEVICE_REGION,
			device_region.dev_class, device_region.dev_instance,
			&device_region, sizeof(device_region));

		++d;
	}
}

static void load_memory_regions(const struct ffa_name_value_pair *value_pair)
{
	/* Not yet supported */
	(void)value_pair;
}

static void load_blob(const struct ffa_name_value_pair *value_pair)
{
	struct config_blob blob;

	blob.data = (const void*)value_pair->value;
	blob.data_len = value_pair->size;

	config_store_add(CONFIG_CLASSIFIER_BLOB,
		(const char *)value_pair->name, 0,
		&blob, sizeof(blob));
}

// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <string.h>
#include <config/interface/platform_config.h>
#include <platform/interface/device_region.h>
#include "sp_config_loader.h"


struct sp_param_device_region
{
	char name[16];
	uintptr_t location;
	size_t size;
};

/**
 * Loads externally provided configuration data originating from
 * theh SP manifest.
 */
void sp_config_load(struct ffa_init_info *init_info)
{
	/* Load deployment specific configuration */
	for (size_t param_index = 0; param_index < init_info->count; param_index++) {

   		if (!strcmp((const char *)init_info->nvp[param_index].name,"DEVICE_REGIONS")) {

			struct sp_param_device_region *d = (struct sp_param_device_region *)init_info->nvp[param_index].value;

            /*Iterate over the device regions*/
			while ((uintptr_t)d < (init_info->nvp[param_index].value + init_info->nvp[param_index].size)) {

				struct device_region device_region;

				strcpy(device_region.dev_class, d->name);
				device_region.dev_instance = 0;
				device_region.base_addr = d->location;
				device_region.io_region_size = d->size;

				platform_config_device_add(&device_region);

				++d;
			}
		}
	}
}

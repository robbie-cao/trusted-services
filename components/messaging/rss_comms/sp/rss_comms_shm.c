/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform/interface/device_region.h>
#include <config/interface/config_store.h>
#include <config/interface/config_blob.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <trace.h>
#include "rss_comms_caller.h"
#include "rss_comms_shm.h"


static int rss_comms_get_shmem(const char *dev,
				     struct device_region *dev_region)
{
	bool found;

	found = config_store_query(CONFIG_CLASSIFIER_DEVICE_REGION, dev, 0,
				   dev_region, sizeof(*dev_region));
	if (!found) {
		EMSG("[RSS-COMMS] shm: device region not found: %s\n", dev);
		return -EINVAL;
	}

	if (dev_region->base_addr == 0 || dev_region->io_region_size == 0) {
		EMSG("[RSS-COMMS] shm: device region not valid\n");
		return -EINVAL;
	}

	IMSG("rss_comms: shm: device region found: %s addr: 0x%lx size: %ld\n",
	     dev, dev_region->base_addr, dev_region->io_region_size);

	return  0;
}

static void rss_comms_shm_set(struct rss_comms_shm *shm,
				   struct device_region *shm_region)
{
	shm->shm_addr = shm_region->base_addr;
	shm->shm_size = shm_region->io_region_size;

	IMSG("SHEM: base: 0x%0lx size: %ld",
	     shm->shm_addr, shm->shm_size);
}

int rss_comms_init(struct rss_comms_messenger *rss_comms_msg)
{
	struct device_region shm_info;
	struct rss_comms_shm *rss_shm;
	int ret;

	if (!rss_comms_msg) {
		EMSG("[RSS-COMMS] rss comms messenger is null\n");
		return -1;
	}

	if (rss_comms_msg->platform) {
		IMSG("[RSS-COMMS] rss comms platform already initialized\n");
		return 0;
	}

	rss_shm = malloc(sizeof(*rss_shm));
	if (!rss_shm) {
		EMSG("[RSS-COMMS] rss comms malloc dev failed\n");
		return -ENOMEM;
	}

	rss_shm->rss_comms_msg = rss_comms_msg;

	ret = rss_comms_get_shmem("rss_comms-shm", &shm_info);
	if (ret < 0) {
		EMSG("[RSS-COMMS] rss comms shared-memory mapping failed");
		goto failed;
	}

	rss_comms_shm_set(rss_shm, &shm_info);

	rss_comms_msg->platform = rss_shm;

	return 0;

failed:
	free(rss_shm);

	return ret;
}

int rss_comms_deinit(struct rss_comms_messenger *rss_comms_msg)
{

	struct rss_comms_shm *rss_shm;

	if (!rss_comms_msg->platform)
		return 0;

	rss_shm = rss_comms_msg->platform;

	free(rss_shm);

	rss_comms_msg->platform = NULL;

	return 0;
}

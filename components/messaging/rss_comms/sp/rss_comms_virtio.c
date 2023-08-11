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
#include "rss_comms_virtio.h"

#define RSS_COMMS_SHEM_PHYS 0xA03F0000


static int rss_comms_virtio_device_get(const char *dev,
				     struct device_region *dev_region)
{
	bool found;

	found = config_store_query(CONFIG_CLASSIFIER_DEVICE_REGION, dev, 0,
				   dev_region, sizeof(*dev_region));
	if (!found) {
		EMSG("[RSS-COMMS] virtio: device region not found: %s\n", dev);
		return -EINVAL;
	}

	if (dev_region->base_addr == 0 || dev_region->io_region_size == 0) {
		EMSG("[RSS-COMMS] virtio: device region not valid\n");
		return -EINVAL;
	}

	IMSG("rss_comms: virtio: device region found: %s addr: 0x%lx size: %ld\n",
	     dev, dev_region->base_addr, dev_region->io_region_size);

	return  0;
}

static void rss_comms_virtio_shm_set(struct rss_comms_virtio *virtio,
				   struct device_region *virtio_region)
{
	struct rss_comms_virtio_shm *shm = &virtio->shm;

	shm->base_addr = virtio_region->base_addr;
	shm->size = virtio_region->io_region_size;

	IMSG("SHEM: base: 0x%0lx size: 0x%0lx size: %ld",
	     shm->base_addr, shm->size, shm->size);
}

int rss_comms_virtio_init(struct rss_comms_messenger *rss_comms_msg)
{
	struct device_region virtio_dev;
	struct rss_comms_virtio *virtio;
	int ret;

	if (!rss_comms_msg) {
		EMSG("[RSS-COMMS] rss comms messenger is null\n");
		return -1;
	}

	if (rss_comms_msg->platform) {
		IMSG("[RSS-COMMS] rss comms platform already initialized\n");
		return 0;
	}

	virtio = malloc(sizeof(*virtio));
	if (!virtio) {
		EMSG("[RSS-COMMS] rss comms malloc virtio failed\n");
		return -ENOMEM;
	}

	virtio->rss_comms_msg = rss_comms_msg;

	ret = rss_comms_virtio_device_get("rss_comms-virtio", &virtio_dev);
	if (ret < 0) {
		EMSG("[RSS-COMMS] rss comms virtio memory mapping failed");
		goto free_virtio;
	}

	rss_comms_virtio_shm_set(virtio, &virtio_dev);

	rss_comms_msg->platform = virtio;

	return 0;

free_virtio:
	free(virtio);

	return ret;
}

int rss_comms_virtio_deinit(struct rss_comms_messenger *rss_comms_msg)
{

	struct rss_comms_virtio *virtio;

	if (!rss_comms_msg->platform)
		return 0;

	virtio = rss_comms_msg->platform;

	free(virtio);

	rss_comms_msg->platform = NULL;

	return 0;
}

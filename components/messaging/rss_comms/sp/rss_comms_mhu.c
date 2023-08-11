/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config/interface/config_store.h>
#include <config/interface/config_blob.h>
#include <platform/interface/device_region.h>
#include <platform/drivers/arm/mhu_driver/mhu_v3_x.h>
#include <platform/drivers/arm/mhu_driver/mhu.h>
#include <trace.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include "rss_comms_caller.h"
#include "rss_comms_mhu.h"
#include "rss_comms_messenger_api.h"

static int rss_comms_mhu_device_get(const char *dev,
				  struct device_region *dev_region)
{
	bool found;

	found = config_store_query(CONFIG_CLASSIFIER_DEVICE_REGION, dev, 0,
				   dev_region, sizeof(*dev_region));
	if (!found)
		return -EINVAL;

	if (!dev_region->base_addr)
		return -EINVAL;

	IMSG("mhu: device region found: %s addr: 0x%lx size: %ld", dev,
	     dev_region->base_addr, dev_region->io_region_size);

	return 0;
}

int rss_comms_mhu_init(struct rss_comms_messenger *rss_comms_msg)
{
	struct mhu_v3_x_dev_t *rx_dev;
	struct mhu_v3_x_dev_t *tx_dev;
	struct rss_comms_mhu *mhu;
	enum mhu_error_t err;
	int ret;

	/* if we already have initialized skip this */
	if (rss_comms_msg->transport) {
		IMSG("[RSS-COMMS] Transport already initialized\n");
		return 0;
	}

	mhu = malloc(sizeof(*mhu));
	if (!mhu) {
		EMSG("[RSS-COMMS] malloc mhu failed\n");
		return -1;
	}

	ret = rss_comms_mhu_device_get("mhu-sender", &mhu->tx_region);
	if (ret < 0)
		goto free_mhu;

	ret = rss_comms_mhu_device_get("mhu-receiver", &mhu->rx_region);
	if (ret < 0)
		goto free_mhu;

	rx_dev = &mhu->rx_dev;
	tx_dev = &mhu->tx_dev;

	rx_dev->base =  (unsigned int)mhu->rx_region.base_addr;
	rx_dev->frame = MHU_V3_X_MBX_FRAME;

	tx_dev->base =  (unsigned int)mhu->tx_region.base_addr;
	tx_dev->frame = MHU_V3_X_PBX_FRAME;

	err = mhu_init_sender(tx_dev);
	if (err != MHU_ERR_NONE) {
		if (err == MHU_ERR_ALREADY_INIT) {
			EMSG("[RSS-COMMS] TS to RSS MHU driver already initialized\n");
		} else {
			EMSG("[RSS-COMMS] TS to RSS MHU driver initialization failed: %d\n", err);
			goto free_mhu;
		}
	}

	err = mhu_init_receiver(rx_dev);
	if (err != MHU_ERR_NONE) {
		if (err == MHU_ERR_ALREADY_INIT) {
			EMSG("[RSS-COMMS] RSS to TS MHU driver already initialized\n");
		} else {
			EMSG("[RSS-COMMS] RSS to TS MHU driver initialization failed: %d\n", err);
			goto free_mhu;
		}
	}

	rss_comms_msg->transport = (void *)mhu;

	return 0;

free_mhu:
	free(mhu);

	return ret;
}

int rss_comms_mhu_deinit(struct rss_comms_messenger *rss_comms_msg)
{
	struct rss_comms_mhu *mhu;

	if (!rss_comms_msg->transport)
		return 0;

	mhu = rss_comms_msg->transport;
	free(mhu);

	rss_comms_msg->transport = NULL;

	return 0;
}

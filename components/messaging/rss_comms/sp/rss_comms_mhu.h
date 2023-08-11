/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_MHU_H__
#define __RSS_COMMS_MHU_H__

#include <stddef.h>
#include "rss_comms_caller.h"
#include <platform/interface/device_region.h>
#include <platform/drivers/arm/mhu_driver/mhu_v3_x.h>
#include "rss_comms_messenger_api.h"

extern struct rss_comms_messenger;

struct rss_comms_mhu {
	struct device_region rx_region;
	struct device_region tx_region;
	struct mhu_v3_x_dev_t rx_dev;
	struct mhu_v3_x_dev_t tx_dev;
};

int rss_comms_mhu_init(struct rss_comms_messenger *rss_comms_msg);
int rss_comms_mhu_deinit(struct rss_comms_messenger *rss_comms_msg);

#endif /* __RSS_COMMS_MHU_H__ */

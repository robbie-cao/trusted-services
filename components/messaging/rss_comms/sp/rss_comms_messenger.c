/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <trace.h>
#include "rss_comms_messenger.h"
#include "rss_comms_messenger_api.h"
#include "rss_comms_mhu.h"
#include <protocols/rpc/common/packed-c/status.h>

static const struct rss_comms_platform_ops rss_comms_ops = {
	.transport_init = rss_comms_mhu_init,
	.transport_deinit = rss_comms_mhu_deinit,
	.platform_init = rss_comms_init,
	.platform_deinit = rss_comms_deinit,
};

int rss_comms_messenger_init(struct rss_comms_messenger *rss_comms_msg)
{
	const struct rss_comms_platform_ops *ops;
	int ret;

	rss_comms_msg->platform_ops = &rss_comms_ops;
	ops = rss_comms_msg->platform_ops;

	ret = ops->transport_init(rss_comms_msg);
	if (ret < 0) {
		EMSG("[RSS-COMMS] transport init failed\n");
		return ret;
	}

	ret = ops->platform_init(rss_comms_msg);
	if (ret < 0) {
		EMSG("[RSS-COMMS] platform init failed\n");
		return ret;
	}
}

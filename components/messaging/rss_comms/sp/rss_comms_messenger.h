/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_MESSENGER_H__
#define __RSS_COMMS_MESSENGER_H__

#include <stddef.h>
#include <stdint.h>
#include "rss_comms_mhu.h"
#include "rss_comms_shm.h"

struct rss_comms_platform_ops {
	int (*transport_init)(struct rss_comms_messenger *rss_comms_msg);
	int (*transport_deinit)(struct rss_comms_messenger *rss_comms_msg);
	int (*platform_init)(struct rss_comms_messenger *rss_comms_msg);
	int (*platform_deinit)(struct rss_comms_messenger *rss_comms_msg);
};

#endif /* __RSS_COMMS_MESSENGER_H__ */

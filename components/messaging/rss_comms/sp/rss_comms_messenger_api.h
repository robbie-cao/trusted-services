/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __RSS_COMMS_MESSENGER_API_H__
#define __RSS_COMMS_MESSENGER_API_H__

#include <stddef.h>
#include <stdint.h>
#include "rss_comms_messenger.h"

struct rss_comms_messenger {
	const struct rss_comms_platform_ops *platform_ops;
	void *transport;
	void *platform;
};

int rss_comms_messenger_init(struct rss_comms_messenger *rss_comms);

#endif /* __RSS_COMMS_MESSENGER_API_H__ */

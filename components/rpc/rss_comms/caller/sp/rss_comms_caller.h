/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_CALLER_H__
#define __RSS_COMMS_CALLER_H__

#include <rpc_caller.h>
#include "rss_comms_protocol.h"
#include "rss_comms_messenger_api.h"

struct rss_comms_caller {
	struct rpc_caller rpc_caller;
	struct rss_comms_messenger rss_comms_msg;
};

struct rpc_caller *rss_comms_caller_init(struct rss_comms_caller *rss_comms);
void rss_comms_caller_deinit(struct rss_comms_caller *rss_comms);

#endif /* __RSS_COMMS_CALLER_H__ */

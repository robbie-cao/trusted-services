/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_MESSENGER_API_H__
#define __RSS_COMMS_MESSENGER_API_H__

#include <stddef.h>
#include <stdint.h>

struct rss_comms_messenger {
	void *msg;
	void *platform;
};

int rss_comms_messenger_init(struct rss_comms_messenger *rss_comms);
void rss_comms_messenger_deinit(struct rss_comms_messenger *rss_comms);
int rss_comms_messenger_call_invoke(struct rss_comms_messenger *rss_comms, uint8_t **resp_buf,
				    size_t *resp_len);
int rss_comms_messenger_call_begin(struct rss_comms_messenger *rss_comms, uint8_t **req_buf,
				   size_t req_len);
void rss_comms_messenger_call_end(struct rss_comms_messenger *rss_comms);

#endif /* __RSS_COMMS_MESSENGER_API_H__ */

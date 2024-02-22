/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_PLATFORM_API_H__
#define __RSS_COMMS_PLATFORM_API_H__

#include <stdint.h>

struct rss_comms_platform;

struct rss_comms_platform *rss_comms_platform_init(void);
int rss_comms_platform_deinit(struct rss_comms_platform *rss_comms_plat);
int rss_comms_platform_invoke(struct rss_comms_platform *rss_comms_plat, uint8_t *resp_buf,
			      uint8_t *req_buf, size_t *resp_len, size_t req_len);
int rss_comms_platform_begin(struct rss_comms_platform *rss_comms_plat, uint8_t *req_buf,
			     size_t req_len);
int rss_comms_platform_end(struct rss_comms_platform *rss_comms_plat);

#endif /* __RSS_COMMS_PLATFORM_API_H__ */

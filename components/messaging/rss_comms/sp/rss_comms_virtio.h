/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_VIRTIO_H__
#define __RSS_COMMS_VIRTIO_H__

#include <stddef.h>
#include "rss_comms_caller.h"

/* Union as message space and reply space are never used at the same time, and this saves space as
 * we can overlap them.
 */
union __packed __attribute__((aligned(4))) rss_comms_io_buffer_t {
	struct serialized_rss_comms_msg_t msg;
	struct serialized_rss_comms_reply_t reply;
};

struct rss_comms_virtio_shm {
	uintptr_t base_addr;
	size_t size;
};

struct rss_comms_virtio {
	struct rss_comms_messenger *rss_comms_msg;
	struct rss_comms_virtio_shm shm;
	union rss_comms_io_buffer_t io_buf;
	size_t msg_size;
};

int rss_comms_virtio_init(struct rss_comms_messenger *rss_comms_msg);
int rss_comms_virtio_deinit(struct rss_comms_messenger *rss_comms_msg);

#endif /* __RSS_COMMS_VIRTIO_H__ */

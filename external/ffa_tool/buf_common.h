/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BUF_COMMON_H
#define __BUF_COMMON_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <uapi/asm-generic/ioctl.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
#endif

struct buf_descr {
	uint8_t *buf;
	size_t length;
};

struct msg_args {
	uint16_t dst_id;
	uint32_t args[5];
};

struct endpoint_id {
	uint32_t uuid_0;
	uint32_t uuid_1;
	uint32_t uuid_2;
	uint32_t uuid_3;
	uint16_t id;
};

#define IOCTL_TYPE	0xf0

#define GET_PART_ID	_IOWR(IOCTL_TYPE, 0x00, struct endpoint_id)
#define SEND_SYNC_MSG	_IOWR(IOCTL_TYPE, 0x01, struct msg_args)

#define SHARE_INIT	_IOR(IOCTL_TYPE, 0x10, unsigned long)
#define SHARE_DEINIT	_IO(IOCTL_TYPE, 0x11)
#define SHARE_READ	_IOW(IOCTL_TYPE, 0x12, struct buf_descr)
#define SHARE_WRITE	_IOW(IOCTL_TYPE, 0x13, struct buf_descr)

#endif /* __BUF_COMMON_H */

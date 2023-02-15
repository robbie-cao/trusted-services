/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/crc32/crc32.h>
#include <common/tf_crc32.h>


uint32_t crc32(uint32_t crc, const unsigned char *buf, size_t size)
{
	return tf_crc32(crc, buf, size);
}

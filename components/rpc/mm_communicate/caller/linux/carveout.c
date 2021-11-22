/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "carveout.h"

/* Need to be aligned with carve-out used by StMM or smm-gateway. */
static const off_t carveout_pa = 0x0000000881000000;
static const size_t carveout_len = 0x8000;

int carveout_claim(uint8_t **buf, size_t *buf_size)
{
	int status = -1;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);

	if (fd >= 0) {

		uint8_t *mem = mmap(NULL, carveout_len,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, carveout_pa);

		if (mem != MAP_FAILED) {

			*buf = mem;
			*buf_size = carveout_len;

			status = 0;
		}

		close(fd);
	}

	return status;
}

void carveout_relinquish(uint8_t *buf, size_t buf_size)
{
	munmap(buf, buf_size);
}

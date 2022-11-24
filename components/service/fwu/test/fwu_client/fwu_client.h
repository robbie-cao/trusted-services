/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWU_CLIENT_H
#define FWU_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <common/uuid/uuid.h>

/*
 * Presents a client interface for interacting with a fwu service provider.
 * Test cases that use this interface can potentially be reused with alternative
 * service provider deployments.
 */
class fwu_client
{
public:

	fwu_client()
	{

	}

	virtual ~fwu_client()
	{

	}

	virtual int begin_staging(void) = 0;

	virtual int end_staging(void) = 0;

	virtual int cancel_staging(void) = 0;

	virtual int accept(
		const struct uuid_octets *image_type_uuid) = 0;

	virtual int select_previous(void) = 0;

	virtual int open(
		const struct uuid_octets *uuid,
		uint32_t *handle) = 0;

	virtual int commit(
		uint32_t handle,
		bool accepted) = 0;

	virtual int write_stream(
		uint32_t handle,
		const uint8_t *data,
		size_t data_len) = 0;

	virtual int read_stream(
		uint32_t handle,
		uint8_t *buf,
		size_t buf_size,
		size_t *read_len,
		size_t *total_len) = 0;
};

#endif /* FWU_CLIENT_H */

/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef REMOTE_FWU_CLIENT_H
#define REMOTE_FWU_CLIENT_H

#include <service/common/client/service_client.h>
#include <service/fwu/test/fwu_client/fwu_client.h>
#include <service_locator.h>

/*
 * An fwu_client that calls a remote fwu service provider via a
 * deployment specific RPC caller. Assumes that call request and
 * response parameters are serialized in-line with the FWU-A specification.
 */
class remote_fwu_client : public fwu_client
{
public:

	remote_fwu_client();
	~remote_fwu_client();

	int begin_staging(void);

	int end_staging(void);

	int cancel_staging(void);

	int accept(
		const struct uuid_octets *image_type_uuid);

	int select_previous(void);

	int open(
		const struct uuid_octets *uuid,
		uint32_t *handle);

	int commit(
		uint32_t handle,
		bool accepted);

	int write_stream(
		uint32_t handle,
		const uint8_t *data,
		size_t data_len);

	int read_stream(
		uint32_t handle,
		uint8_t *buf,
		size_t buf_size,
		size_t *read_len,
		size_t *total_len);

private:

	int invoke_no_param(unsigned int opcode);
	void open_session(void);
	void close_session(void);

	struct service_client m_client;
	rpc_session_handle m_rpc_session_handle;
	struct service_context *m_service_context;
};

#endif /* REMOTE_FWU_CLIENT_H */

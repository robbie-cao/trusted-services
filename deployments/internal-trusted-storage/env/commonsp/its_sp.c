/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "components/rpc/common/endpoint/rpc_interface.h"
#include "components/rpc/ffarpc/endpoint/ffarpc_call_ep.h"
#include "components/service/secure_storage/factory/storage_factory.h"
#include "components/service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h"
#include "sp_api.h"
#include "sp_discovery.h"
#include "sp_messaging.h"
#include "sp_rxtx.h"
#include "trace.h"

static uint8_t tx_buffer[4096] __aligned(4096);
static uint8_t rx_buffer[4096] __aligned(4096);

void sp_main(struct ffa_init_info *init_info)
{
	sp_result result = SP_RESULT_INTERNAL_ERROR;
	struct rpc_interface *secure_storage_iface = NULL;
	struct ffa_call_ep ffa_call_ep = { 0 };
	struct sp_msg req_msg = { 0 };
	struct sp_msg resp_msg = { 0 };
	struct secure_storage_provider secure_storage_provider = { 0 };
	struct storage_backend *storage_backend = NULL;
	uint16_t own_id = 0;

	/* Boot */
	(void) init_info;

	result = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (result != SP_RESULT_OK) {
		EMSG("Failed to map RXTX buffers: %d", result);
		goto fatal_error;
	}

	result = sp_discovery_own_id_get(&own_id);
	if (result != SP_RESULT_OK) {
		EMSG("Failed to query own ID: %d", result);
		goto fatal_error;
	}

	storage_backend = storage_factory_create(storage_factory_security_class_INTERNAL_TRUSTED);
	if (!storage_backend) {
		EMSG("Failed to create storage backend");
		goto fatal_error;
	}

	secure_storage_iface = secure_storage_provider_init(&secure_storage_provider, storage_backend);
	if (!secure_storage_iface) {
		EMSG("Failed to init secure storage provider");
		goto fatal_error;
	}

	ffa_call_ep_init(&ffa_call_ep, secure_storage_iface, own_id);

	/* End of boot phase */
	result = sp_msg_wait(&req_msg);
	if (result != SP_RESULT_OK) {
		EMSG("Failed to send message wait %d", result);
		goto fatal_error;
	}

	while (1) {
		ffa_call_ep_receive(&ffa_call_ep, &req_msg, &resp_msg);

		result = sp_msg_send_direct_resp(&resp_msg, &req_msg);
		if (result != SP_RESULT_OK) {
			EMSG("Failed to send direct response %d", result);
			result = sp_msg_wait(&req_msg);
			if (result != SP_RESULT_OK) {
				EMSG("Failed to send message wait %d", result);
				goto fatal_error;
			}
		}
	}

fatal_error:
	/* SP is not viable */
	EMSG("ITS SP error");
	while (1) {}
}

void sp_interrupt_handler(uint32_t interrupt_id)
{
	(void)interrupt_id;
}

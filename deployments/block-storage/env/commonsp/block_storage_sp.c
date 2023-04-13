// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 */

#include "common/crc32/crc32.h"
#include "rpc/ffarpc/endpoint/ffarpc_call_ep.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "config/ramstore/config_ramstore.h"
#include "config/loader/sp/sp_config_loader.h"
#include "service/block_storage/provider/block_storage_provider.h"
#include "service/block_storage/provider/serializer/packed-c/packedc_block_storage_serializer.h"
#include "service/block_storage/factory/block_store_factory.h"
#include "sp_api.h"
#include "sp_discovery.h"
#include "sp_messaging.h"
#include "sp_rxtx.h"
#include "trace.h"


static bool sp_init(uint16_t *own_sp_id);

void __noreturn sp_main(union ffa_boot_info *boot_info)
{
	struct ffa_call_ep ffarpc_call_ep = { 0 };
	struct block_storage_provider service_provider = { 0 };
	struct block_store *backend = NULL;
	struct rpc_interface *service_iface = NULL;
	struct sp_msg req_msg = { 0 };
	struct sp_msg resp_msg = { 0 };
	uint16_t own_id = 0;
	sp_result result = SP_RESULT_INTERNAL_ERROR;

	/* Boot phase */
	if (!sp_init(&own_id)) {
		EMSG("Failed to init SP");
		goto fatal_error;
	}

	config_ramstore_init();

	if (!sp_config_load(boot_info)) {
		EMSG("Failed to load SP config");
		goto fatal_error;
	}

	crc32_init();

	/* Initialise the service provider and backend block store */
	backend = block_store_factory_create();
	if (!backend) {
		EMSG("Failed to create block store");
		goto fatal_error;
	}

	service_iface = block_storage_provider_init(
		&service_provider,
		backend);
	if (!service_iface) {
		EMSG("Failed to init service provider");
		goto fatal_error;
	}

	block_storage_provider_register_serializer(
		&service_provider,
		TS_RPC_ENCODING_PACKED_C,
		packedc_block_storage_serializer_instance());

	/* Associate service interface with FFA call endpoint */
	ffa_call_ep_init(&ffarpc_call_ep, service_iface, own_id);

	/* End of boot phase */
	result = sp_msg_wait(&req_msg);
	if (result != SP_RESULT_OK) {
		EMSG("Failed to send message wait %d", result);
		goto fatal_error;
	}

	while (1) {
		ffa_call_ep_receive(&ffarpc_call_ep, &req_msg, &resp_msg);

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
	EMSG("Block storage SP error");
	while (1) {}
}

void sp_interrupt_handler(uint32_t interrupt_id)
{
	(void)interrupt_id;
}

static bool sp_init(uint16_t *own_id)
{
	sp_result sp_res = SP_RESULT_INTERNAL_ERROR;
	static uint8_t tx_buffer[4096] __aligned(4096);
	static uint8_t rx_buffer[4096] __aligned(4096);

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res != SP_RESULT_OK) {
		EMSG("Failed to map RXTX buffers: %d", sp_res);
		return false;
	}

	sp_res = sp_discovery_own_id_get(own_id);
	if (sp_res != SP_RESULT_OK) {
		EMSG("Failed to query own ID: %d", sp_res);
		return false;
	}

	return true;
}

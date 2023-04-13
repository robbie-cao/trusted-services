// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 */

#include <stddef.h>

#include "common/crc32/crc32.h"
#include "config/loader/sp/sp_config_loader.h"
#include "config/ramstore/config_ramstore.h"
#include "media/volume/factory/volume_factory.h"
#include "protocols/rpc/common/packed-c/status.h"
#include "rpc/ffarpc/endpoint/ffarpc_call_ep.h"
#include "service/discovery/provider/discovery_provider.h"
#include "service/discovery/provider/serializer/packed-c/packedc_discovery_provider_serializer.h"
#include "service/fwu/agent/update_agent.h"
#include "service/fwu/config/fwu_configure.h"
#include "service/fwu/fw_store/banked/bank_scheme.h"
#include "service/fwu/fw_store/banked/banked_fw_store.h"
#include "service/fwu/fw_store/banked/metadata_serializer/v1/metadata_serializer_v1.h"
#include "service/fwu/fw_store/banked/metadata_serializer/v2/metadata_serializer_v2.h"
#include "service/fwu/inspector/direct/direct_fw_inspector.h"
#include "service/fwu/provider/fwu_provider.h"
#include "service/fwu/provider/serializer/packed-c/packedc_fwu_provider_serializer.h"
#include "sp_api.h"
#include "sp_discovery.h"
#include "sp_messaging.h"
#include "sp_rxtx.h"
#include "trace.h"

/* Set default limit on the number of storage devices to update */
#ifndef FWU_SP_MAX_STORAGE_DEVICES
#define FWU_SP_MAX_STORAGE_DEVICES (1)
#endif

/* Parameters that should be passed forward by the bootloader */
#define HARD_CODED_BOOT_INDEX	(0)
#define HARD_CODED_METADATA_VER (2)

static bool sp_init(uint16_t *own_sp_id);
static bool configure_for_platform(void);
const struct metadata_serializer *select_metadata_serializer(unsigned int version);

void __noreturn sp_main(union ffa_boot_info *boot_info)
{
	struct ffa_call_ep ffarpc_call_ep = { 0 };
	struct fwu_provider service_provider = { 0 };
	struct rpc_interface *service_iface = NULL;
	struct update_agent update_agent = { 0 };
	struct fw_store fw_store = { 0 };
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

	/* Configuration - discovers required volumes and installers */
	if (!configure_for_platform()) {
		EMSG("Failed to configure for platform");
		goto fatal_error;
	}

	/* Select FWU metadata serializer for compatibility with bootloader */
	const struct metadata_serializer *serializer =
		select_metadata_serializer(HARD_CODED_METADATA_VER);

	if (!serializer) {
		EMSG("Unsupported FWU metadata version");
		goto fatal_error;
	}

	/* Initialise fw store */
	if (banked_fw_store_init(&fw_store, serializer)) {
		EMSG("Failed to init fw store");
		goto fatal_error;
	}

	if (update_agent_init(&update_agent, HARD_CODED_BOOT_INDEX, direct_fw_inspector_inspect,
			      &fw_store)) {
		EMSG("Failed to init update agent");
		goto fatal_error;
	}

	/* Initialise the FWU service provider */
	service_iface = fwu_provider_init(&service_provider, &update_agent);

	if (!service_iface) {
		EMSG("Failed to init service provider");
		goto fatal_error;
	}

	fwu_provider_register_serializer(&service_provider, TS_RPC_ENCODING_PACKED_C,
					 packedc_fwu_provider_serializer_instance());

	discovery_provider_register_serializer(&service_provider.discovery_provider,
					       TS_RPC_ENCODING_PACKED_C,
					       packedc_discovery_provider_serializer_instance());

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
	EMSG("FWU SP error");
	while (1) {
	}
}

void sp_interrupt_handler(uint32_t interrupt_id)
{
	(void)interrupt_id;
}

static bool sp_init(uint16_t *own_id)
{
	static uint8_t tx_buffer[4096] __aligned(4096);
	static uint8_t rx_buffer[4096] __aligned(4096);

	sp_result sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
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

static bool configure_for_platform(void)
{
	struct uuid_octets device_uuids[FWU_SP_MAX_STORAGE_DEVICES];
	size_t num_storage_devices = 0;

	int status =
		volume_factory_init(device_uuids, FWU_SP_MAX_STORAGE_DEVICES, &num_storage_devices);

	if (status) {
		EMSG("Failed to init volume factory: %d", status);
		return false;
	}

	status = fwu_configure(device_uuids, num_storage_devices);

	if (status) {
		EMSG("Failed to setup FWU configuration: %d", status);
		return false;
	}

	return true;
}

const struct metadata_serializer *select_metadata_serializer(unsigned int version)
{
	if (version == 1)
		return metadata_serializer_v1();

	if (version == 2)
		return metadata_serializer_v2();

	return NULL;
}
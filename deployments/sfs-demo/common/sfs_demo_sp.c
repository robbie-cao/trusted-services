/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ffa_api.h>
#include <components/rpc/ffarpc/caller/sp/ffarpc_caller.h>
#include <components/service/secure_storage/frontend/psa/its/its_frontend.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <psa/internal_trusted_storage.h>
#include <sp_api.h>
#include "sp_messaging.h"
#include <sp_rxtx.h>
#include <trace.h>
#include <string.h>

// #define SP_ITS_UUID_BYTES \
// 	{ 0xdc, 0x1e, 0xef, 0x48, 0xb1, 0x7a, 0x4c, 0xcf, \
// 	  0xac, 0x8b, 0xdf, 0xcf, 0xf7, 0x71, 0x1b, 0x14, }

#define SP_ITS_UUID_BYTES \
	{ 0x48, 0xef, 0x1e, 0xdc, 0x7a, 0xb1, 0xcf, 0x4c, \
	  0xac, 0x8b, 0xdf, 0xcf, 0xf7, 0x71, 0x1b, 0x14, }

static uint8_t tx_buffer[4096] __aligned(4096);
static uint8_t rx_buffer[4096] __aligned(4096);
static const uint8_t its_uuid[] = SP_ITS_UUID_BYTES;

static const psa_storage_uid_t test_data_uid = 0x12345678;
static const uint8_t test_data[] = {
	0xc3, 0xe2, 0xf8, 0x1c, 0xe0, 0x87, 0x8a, 0x14, 0xbf, 0x59, 0xa3, 0xff,
	0x96, 0x50, 0x25, 0x95, 0x76, 0xdc, 0xbe, 0xe6, 0x45, 0x45, 0x1d, 0x1b,
	0x34, 0x6a, 0xa1, 0x1c, 0xba, 0x24, 0xa9, 0x82, 0xf1, 0x03, 0x30, 0x9b,
	0x7d, 0xf6, 0x30, 0x88, 0xc2, 0xfb, 0xd7, 0x43, 0xfa, 0x82, 0x7c, 0x30,
	0x49, 0x71, 0xcb, 0xe6, 0xf8, 0x18, 0xb9, 0xfc, 0x61, 0x92, 0x8c, 0x86,
	0x01, 0x3a, 0x4a, 0xba, 0x88, 0x58, 0x63, 0x27, 0x9a, 0x47, 0xd3, 0x10,
	0xbf, 0x80, 0x70, 0x19, 0xab, 0xc3, 0x88, 0xdb, 0xc3, 0x0c, 0x6e, 0xe5,
	0xb1, 0xed, 0x9f, 0x47, 0xd4, 0x02, 0xa3, 0xb5, 0xb5, 0x7a, 0x20, 0x48,
	0xd5, 0x78, 0x95, 0x7d, 0xb8, 0x33, 0xb0, 0xad, 0x4a, 0x17, 0x5a, 0xff,
	0xd0, 0x12, 0x32, 0x84, 0x9a, 0xa3, 0x98, 0xd9, 0x26, 0x47, 0x55, 0xb7,
	0x31, 0x96, 0x1f, 0x89, 0xd7, 0xe1, 0x01, 0x5a, 0x71, 0x6d, 0xc1, 0xe2,
	0x26, 0x98, 0x12, 0x71, 0x4f, 0xa1, 0xdb, 0x50, 0xc4, 0xc6, 0x2f, 0xed,
	0x5b, 0xf1, 0x52, 0x97, 0x8e, 0xdb, 0xeb, 0x56, 0x41, 0xf5, 0x9c, 0xbf,
	0x6c, 0xe2, 0x1b, 0x14, 0x23, 0xb6, 0x1d, 0x68, 0x0b, 0x70, 0xa4, 0xc8,
	0x70, 0x9f, 0x0a, 0x65, 0x83, 0x27, 0xd5, 0xbb, 0x4b, 0x7d, 0x55, 0x25,
	0xe1, 0x9a, 0xaa, 0x10, 0x5c, 0x49, 0x8f, 0x0f, 0xee, 0x61, 0x49, 0x70,
	0xee, 0x55, 0x46, 0xec, 0x8b, 0x52, 0xf6, 0x65, 0x28, 0x7f, 0x56, 0x7a,
	0xe2, 0xb3, 0xd2, 0xbf, 0xc3, 0x0c, 0x06, 0x8e, 0x5f, 0xdc, 0xd3, 0x1f,
	0x85, 0x74, 0x38, 0x96, 0x5e, 0x1b, 0xe5, 0xa0, 0xc0, 0xfb, 0x90, 0xb7,
	0x14, 0x16, 0x1b, 0xbe, 0xd8, 0x5c, 0x8e, 0xcc, 0x74, 0x71, 0x8c, 0x34,
	0xc0, 0xbc, 0x24, 0xf5, 0xb9, 0x9b, 0xa1, 0x59, 0xe8, 0x54, 0xd0, 0xe9,
	0x01, 0xa9, 0x6e, 0x27,
};
static const size_t test_data_size = sizeof(test_data);
static uint8_t check_data[256] = { 0 };
static size_t check_data_size = 0;

static void run_its_test(void)
{
	psa_status_t its_status;
	struct psa_storage_info_t info;

	/* Write data */
	its_status = psa_its_set(test_data_uid, test_data_size, test_data,
				 PSA_STORAGE_FLAG_NONE);

	if (its_status != PSA_SUCCESS) {
		EMSG("Error: %d", its_status);
	}

	/* Read back and compare */
	its_status = psa_its_get(test_data_uid, 0, test_data_size,
				 check_data, &check_data_size);

	if (its_status != PSA_SUCCESS || check_data_size != test_data_size) {
		EMSG("Error: %d", its_status);
	}

	if(memcmp(test_data, check_data, test_data_size)) {
		EMSG("Data check failed");
	}

	memset(check_data, 0, test_data_size);

	/* Get info of entry */
	psa_its_get_info(test_data_uid, &info);
	if (its_status != PSA_SUCCESS) {
		EMSG("Error: %d", its_status);
	}

	/* Write data with new uid */
	its_status = psa_its_set(test_data_uid + 1, test_data_size, test_data,
				 PSA_STORAGE_FLAG_NONE);

	if (its_status != PSA_SUCCESS) {
		EMSG("Error: %d", its_status);
	}

	/* Read back and compare */
	its_status = psa_its_get(test_data_uid + 1, 0, test_data_size,
				 check_data, &check_data_size);

	if (its_status != PSA_SUCCESS || check_data_size != test_data_size) {
		EMSG("Error: %d", its_status);
	}

	if(memcmp(test_data, check_data, test_data_size)) {
		EMSG("Data check failed");
	}

	/* Delete entry */
	its_status = psa_its_remove(test_data_uid + 1);
	if (its_status != PSA_SUCCESS) {
		EMSG("Error: %d", its_status);
	}

	/* Check if really deleted */
	its_status = psa_its_get(test_data_uid + 1, 0, test_data_size,
				 check_data, &check_data_size);

	if (its_status != PSA_ERROR_DOES_NOT_EXIST) {
		EMSG("Error: %d", its_status);
	}

	IMSG("ITS test done");
}

void __noreturn sp_main(struct ffa_init_info *init_info) {

	ffa_result ffa_res;
	sp_result sp_res;
	struct sp_msg req_msg = { 0 };
	struct rpc_caller *caller;
	struct ffarpc_caller ffa_caller;
	struct secure_storage_client secure_storage_client;
	struct storage_backend *storage_backend;
	uint16_t sp_ids[3];
	uint32_t sp_id_cnt = 0;
	uint16_t own_id = 0;

	/* Boot */
	(void) init_info;
	IMSG("Test SP started");

	sp_res = sp_rxtx_buffer_map(tx_buffer, rx_buffer, sizeof(rx_buffer));
	if (sp_res != SP_RESULT_OK) {
		goto err;
	}

	ffa_res = ffa_id_get(&own_id);
	if (ffa_res != FFA_OK) {
		goto err;
	}
	IMSG("Test SP ID: 0x%x", own_id);

	caller = ffarpc_caller_init(&ffa_caller, own_id);
	sp_id_cnt = ffarpc_caller_discover(its_uuid, sp_ids, 3);

	if (sp_id_cnt == 0) {
		EMSG("Error: %d", sp_id_cnt);
		goto err;
	}
	IMSG("ITS SP ID: 0x%x", sp_ids[0]);

	if (ffarpc_caller_open(&ffa_caller, sp_ids[0], 0)) {
		goto err;
	}

	storage_backend = secure_storage_client_init(&secure_storage_client, caller);
	psa_its_frontend_init(storage_backend);

	/*
	 * This is not thorough testing of the ITS SP!
	 * Only some basic functionality checks.
	 */
	run_its_test();

	if (ffarpc_caller_close(&ffa_caller)) {
		goto err;
	}

	/* End of boot phase */
	sp_msg_wait(&req_msg);

err:
	EMSG("Test SP error");
	while (1) {}
}

void sp_interrupt_handler(uint32_t interrupt_id) {
	(void)interrupt_id;
}

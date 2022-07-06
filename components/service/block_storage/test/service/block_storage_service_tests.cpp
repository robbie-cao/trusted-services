/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstring>
#include "common/uuid/uuid.h"
#include "service/block_storage/block_store/client/block_storage_client.h"
#include "service/block_storage/config/ref/ref_partition_configurator.h"
#include "protocols/rpc/common/packed-c/encoding.h"
#include "service_locator.h"
#include "CppUTest/TestHarness.h"

/**
 * Service-level tests for the block storage service.
 *
 * Test cases assume that the backend block store has been configured using
 * the reference configuration defined in ref_partition_configurator.c. This
 * includes a set of partitions, identified by GUIDs referenced by these test
 * cases.
 */
TEST_GROUP(BlockStorageServiceTests)
{
	void setup()
	{
		struct rpc_caller *caller;
		int status;

		m_rpc_session_handle = NULL;
		m_service_context = NULL;

		service_locator_init();

		m_service_context =
			service_locator_query("sn:trustedfirmware.org:block-storage:0", &status);
		CHECK_TRUE(m_service_context);

		m_rpc_session_handle =
			service_context_open(m_service_context, TS_RPC_ENCODING_PACKED_C, &caller);
		CHECK_TRUE(m_rpc_session_handle);

		m_block_store = block_storage_client_init(&m_client, caller);
		CHECK_TRUE(m_block_store);

		uuid_parse_to_octets(REF_PARTITION_1_GUID,
			m_partition_1_guid.octets, sizeof(m_partition_1_guid.octets));
		uuid_parse_to_octets(REF_PARTITION_2_GUID,
			m_partition_2_guid.octets, sizeof(m_partition_2_guid.octets));
		uuid_parse_to_octets(REF_PARTITION_3_GUID,
			m_partition_3_guid.octets, sizeof(m_partition_3_guid.octets));
		uuid_parse_to_octets(REF_PARTITION_4_GUID,
			m_partition_4_guid.octets, sizeof(m_partition_4_guid.octets));
	}

	void teardown()
	{
		block_storage_client_deinit(&m_client);

		if (m_service_context) {

			if (m_rpc_session_handle) {
				service_context_close(m_service_context, m_rpc_session_handle);
				m_rpc_session_handle = NULL;
			}

			service_context_relinquish(m_service_context);
			m_service_context = NULL;
		}
	}

	static const uint32_t LOCAL_CLIENT_ID = 1;

	rpc_session_handle m_rpc_session_handle;
	struct service_context *m_service_context;
	struct block_storage_client m_client;
	struct block_store *m_block_store;
	struct uuid_octets m_partition_1_guid;
	struct uuid_octets m_partition_2_guid;
	struct uuid_octets m_partition_3_guid;
	struct uuid_octets m_partition_4_guid;
};

TEST(BlockStorageServiceTests, getPartitionInfo)
{
	struct storage_partition_info info;

	/* Check partition info for partition 1 */
	psa_status_t status = block_store_get_partition_info(
		m_block_store, &m_partition_1_guid, &info);

	LONGS_EQUAL(PSA_SUCCESS, status);
	UNSIGNED_LONGS_EQUAL(
		REF_PARTITION_1_ENDING_LBA - REF_PARTITION_1_STARTING_LBA + 1,
		info.num_blocks);
	UNSIGNED_LONGS_EQUAL(REF_PARTITION_BLOCK_SIZE, info.block_size);

	/* Check partition info for partition 2 */
	status = block_store_get_partition_info(
		m_block_store, &m_partition_2_guid, &info);

	LONGS_EQUAL(PSA_SUCCESS, status);
	UNSIGNED_LONGS_EQUAL(
		REF_PARTITION_2_ENDING_LBA - REF_PARTITION_2_STARTING_LBA + 1,
		info.num_blocks);
	UNSIGNED_LONGS_EQUAL(REF_PARTITION_BLOCK_SIZE, info.block_size);

	/* Check partition info for partition 3 */
	status = block_store_get_partition_info(
		m_block_store, &m_partition_3_guid, &info);

	LONGS_EQUAL(PSA_SUCCESS, status);
	UNSIGNED_LONGS_EQUAL(
		REF_PARTITION_3_ENDING_LBA - REF_PARTITION_3_STARTING_LBA + 1,
		info.num_blocks);
	UNSIGNED_LONGS_EQUAL(REF_PARTITION_BLOCK_SIZE, info.block_size);

	/* Check partition info for partition 4 */
	status = block_store_get_partition_info(
		m_block_store, &m_partition_4_guid, &info);

	LONGS_EQUAL(PSA_SUCCESS, status);
	UNSIGNED_LONGS_EQUAL(
		REF_PARTITION_4_ENDING_LBA - REF_PARTITION_4_STARTING_LBA + 1,
		info.num_blocks);
	UNSIGNED_LONGS_EQUAL(REF_PARTITION_BLOCK_SIZE, info.block_size);
}

TEST(BlockStorageServiceTests, openClose)
{
	storage_partition_handle_t handle_1;
	storage_partition_handle_t handle_2;
	storage_partition_handle_t handle_3;
	storage_partition_handle_t handle_4;

	/* Open all reference partitions */
	psa_status_t status = block_store_open(
		m_block_store, LOCAL_CLIENT_ID, &m_partition_1_guid, &handle_1);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = block_store_open(
		m_block_store, LOCAL_CLIENT_ID, &m_partition_2_guid, &handle_2);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = block_store_open(
		m_block_store, LOCAL_CLIENT_ID, &m_partition_3_guid, &handle_3);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = block_store_open(
		m_block_store, LOCAL_CLIENT_ID, &m_partition_4_guid, &handle_4);
	LONGS_EQUAL(PSA_SUCCESS, status);

	/* Close them all */
	status = block_store_close(m_block_store, LOCAL_CLIENT_ID, handle_1);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = block_store_close(m_block_store, LOCAL_CLIENT_ID, handle_2);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = block_store_close(m_block_store, LOCAL_CLIENT_ID, handle_3);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = block_store_close(m_block_store, LOCAL_CLIENT_ID, handle_4);
	LONGS_EQUAL(PSA_SUCCESS, status);
}

TEST(BlockStorageServiceTests, blockAccessOperations)
{
	storage_partition_handle_t handle;
	uint8_t write_buffer[REF_PARTITION_BLOCK_SIZE];
	uint8_t read_buffer[REF_PARTITION_BLOCK_SIZE];
	struct storage_partition_info info;
	size_t num_written = 0;

	/* Get info about one of the partitions */
	psa_status_t status = block_store_get_partition_info(
		m_block_store, &m_partition_3_guid, &info);
	LONGS_EQUAL(PSA_SUCCESS, status);

	/* Open a partition */
	status = block_store_open(
		m_block_store, LOCAL_CLIENT_ID, &m_partition_3_guid, &handle);
	LONGS_EQUAL(PSA_SUCCESS, status);

	/* Write over the entire partition */
	for (uint32_t lba = 0; lba < info.num_blocks; ++lba) {

		memset(write_buffer, lba & 0xff, sizeof(write_buffer));

		status = block_store_write(
			m_block_store, LOCAL_CLIENT_ID, handle, lba,
			0, write_buffer, sizeof(write_buffer), &num_written);
		LONGS_EQUAL(PSA_SUCCESS, status);
		UNSIGNED_LONGS_EQUAL(sizeof(write_buffer), num_written);
	}

	/* Expect to read the same data back */
	for (uint32_t lba = 0; lba < info.num_blocks; ++lba) {

		size_t data_len;

		memset(write_buffer, lba & 0xff, sizeof(write_buffer));
		memset(read_buffer, 0, sizeof(read_buffer));

		status = block_store_read(
			m_block_store, LOCAL_CLIENT_ID, handle, lba,
			0, sizeof(read_buffer), read_buffer, &data_len);
		LONGS_EQUAL(PSA_SUCCESS, status);
		UNSIGNED_LONGS_EQUAL(sizeof(write_buffer), data_len);
		MEMCMP_EQUAL(write_buffer, read_buffer, sizeof(write_buffer));
	}

	/* Write again to the first block - this should fail as it's not erased */
	status = block_store_write(
		m_block_store, LOCAL_CLIENT_ID, handle, 0,
		0, write_buffer, sizeof(write_buffer), &num_written);
	LONGS_EQUAL(PSA_ERROR_STORAGE_FAILURE, status);

	/* Erase a set of blocks */
	uint32_t erase_begin_lba = 0;
	size_t erase_len = 3;
	status = block_store_erase(
		m_block_store, LOCAL_CLIENT_ID, handle,
		erase_begin_lba, erase_len);
	LONGS_EQUAL(PSA_SUCCESS, status);

	/* Write over the erased blocks - this should now work */
	for (uint32_t lba = erase_begin_lba; lba < erase_len; ++lba) {

		memset(write_buffer, lba & 0xff, sizeof(write_buffer));

		status = block_store_write(
			m_block_store, LOCAL_CLIENT_ID, handle, lba,
			0, write_buffer, sizeof(write_buffer), &num_written);
		LONGS_EQUAL(PSA_SUCCESS, status);
		UNSIGNED_LONGS_EQUAL(sizeof(write_buffer), num_written);
	}

	/* Expect to successfully close the partition*/
	status = block_store_close(m_block_store, LOCAL_CLIENT_ID, handle);
	LONGS_EQUAL(PSA_SUCCESS, status);
}

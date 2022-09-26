/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <string>
#include <cstring>
#include <common/uuid/uuid.h>
#include <service/block_storage/block_store/device/ram/ram_block_store.h>
#include <media/volume/block_io_dev/block_io_dev.h>
#include <media/volume/index/volume_index.h>
#include <media/volume/base_io_dev/base_io_dev.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(BlockIoDevTests)
{
	void setup()
	{
		uuid_parse_to_octets("6152f22b-8128-4c1f-981f-3bd279519907",
			m_partition_guid.octets, sizeof(m_partition_guid.octets));

		m_block_store = ram_block_store_init(&m_ram_block_store,
			&m_partition_guid, NUM_BLOCKS, BLOCK_SIZE, NULL);

		CHECK_TRUE(m_block_store);

		m_dev_handle = 0;
		m_volume_spec = 0;

		int result = block_io_dev_init(&m_block_io_dev,
			m_block_store, &m_partition_guid,
			&m_dev_handle, &m_volume_spec);

		LONGS_EQUAL(0, result);
		CHECK_TRUE(m_dev_handle);
		CHECK_TRUE(m_volume_spec);

		volume_index_init();
		volume_index_add(TEST_VOLUME_ID, m_dev_handle, m_volume_spec);
	}

	void teardown()
	{
		block_io_dev_deinit(&m_block_io_dev);
		ram_block_store_deinit(&m_ram_block_store);
		volume_index_clear();
	}

	static const unsigned int TEST_VOLUME_ID = 5;
	static const size_t NUM_BLOCKS = 100;
	static const size_t BLOCK_SIZE = 512;

	struct uuid_octets m_partition_guid;
	struct block_store *m_block_store;
	struct ram_block_store m_ram_block_store;
	struct block_io_dev m_block_io_dev;
	uintptr_t m_dev_handle;
	uintptr_t m_volume_spec;
};


TEST(BlockIoDevTests, openClose)
{
	/* Check the open flow used by tf-a components */
	uintptr_t dev_handle = 0;
	uintptr_t volume_spec = 0;
	uintptr_t file_handle = 0;
	int result;

	result = plat_get_image_source(TEST_VOLUME_ID, &dev_handle, &volume_spec);
	LONGS_EQUAL(0, result);
	CHECK_TRUE(dev_handle);

	result = io_open(dev_handle, volume_spec, &file_handle);
	LONGS_EQUAL(0, result);
	CHECK_TRUE(file_handle);

	io_close(file_handle);
}

TEST(BlockIoDevTests, readAndWrite)
{
	uintptr_t file_handle = 0;
	int result;

	result = io_open(m_dev_handle, m_volume_spec, &file_handle);
	LONGS_EQUAL(0, result);
	CHECK_TRUE(file_handle);

	std::string message("Oh what a beautiful mornin'");

	/* Ensure writes cross a block boundary */
	size_t num_iterations = BLOCK_SIZE / message.size() + 2;

	/* Write message a few times. Expect file pointer to advance on each write */
	for (size_t i = 0; i < num_iterations; ++i) {

		size_t len_written = 0;

		result = io_write(file_handle,
			(const uintptr_t)message.c_str(), message.size(),
			&len_written);

		LONGS_EQUAL(0, result);
		UNSIGNED_LONGS_EQUAL(message.size(), len_written);
	}

	result = io_seek(file_handle, IO_SEEK_SET, 0);
	LONGS_EQUAL(0, result);

	/* Expect to read back the same data */
	uint8_t read_buf[message.size()];

	for (size_t i = 0; i < num_iterations; ++i) {

		size_t len_read = 0;

		memset(read_buf, 0, sizeof(read_buf));

		result = io_read(file_handle,
			(const uintptr_t)read_buf, sizeof(read_buf),
			&len_read);

		LONGS_EQUAL(0, result);
		UNSIGNED_LONGS_EQUAL(message.size(), len_read);
		MEMCMP_EQUAL(message.c_str(), read_buf, message.size());
	}

	io_close(file_handle);
}

TEST(BlockIoDevTests, seekAccess)
{
	uintptr_t file_handle = 0;
	size_t len = 0;
	int result;

	result = io_open(m_dev_handle, m_volume_spec, &file_handle);
	LONGS_EQUAL(0, result);
	CHECK_TRUE(file_handle);

	std::string message("Knees up Mother Brown");

	/* Initially seek to an arbitrary position around the middle of the volume */
	size_t start_pos = (NUM_BLOCKS * BLOCK_SIZE) / 2 + 27;

	/* Seek and write a few times */
	result = io_seek(file_handle, IO_SEEK_SET, start_pos);
	LONGS_EQUAL(0, result);

	result = io_write(file_handle, (const uintptr_t)message.c_str(), message.size(), &len);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(message.size(), len);

	/* Using IO_SEEK_SET, seek forward, skipping over the written message */
	result = io_seek(file_handle, IO_SEEK_SET, start_pos + 110);
	LONGS_EQUAL(0, result);

	result = io_write(file_handle, (const uintptr_t)message.c_str(), message.size(), &len);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(message.size(), len);

	/* Using IO_SEEK_CUR, seek forward again, far enough to skip over the message */
	result = io_seek(file_handle, IO_SEEK_CUR, 715);
	LONGS_EQUAL(0, result);

	result = io_write(file_handle, (const uintptr_t)message.c_str(), message.size(), &len);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(message.size(), len);

	/* Perform the same sequence of seeks and expect to read back intact copies of the message */
	uint8_t read_buf[message.size()];

	result = io_seek(file_handle, IO_SEEK_SET, start_pos);
	LONGS_EQUAL(0, result);

	result = io_read(file_handle, (uintptr_t)read_buf, sizeof(read_buf), &len);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(message.size(), len);
	MEMCMP_EQUAL(message.c_str(), read_buf, message.size());

	result = io_seek(file_handle, IO_SEEK_SET, start_pos + 110);
	LONGS_EQUAL(0, result);

	result = io_read(file_handle, (uintptr_t)read_buf, sizeof(read_buf), &len);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(message.size(), len);
	MEMCMP_EQUAL(message.c_str(), read_buf, message.size());

	result = io_seek(file_handle, IO_SEEK_CUR, 715);
	LONGS_EQUAL(0, result);

	result = io_read(file_handle, (uintptr_t)read_buf, sizeof(read_buf), &len);
	LONGS_EQUAL(0, result);
	UNSIGNED_LONGS_EQUAL(message.size(), len);
	MEMCMP_EQUAL(message.c_str(), read_buf, message.size());

	io_close(file_handle);
}
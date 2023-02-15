/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <drivers/io/io_storage.h>
#include "block_io_dev.h"

/* Concrete io_dev interface functions */
static io_type_t block_io_dev_type(
	void);
static int block_io_dev_open(
	io_dev_info_t *dev_info, const uintptr_t spec, io_entity_t *entity);
static int block_io_dev_close(
	io_entity_t *entity);
static int block_io_dev_seek(
	io_entity_t *entity, int mode, signed long long offset);
static int block_io_dev_size(
	io_entity_t *entity, size_t *length);
static int block_io_dev_read(
	io_entity_t *entity, uintptr_t buffer, size_t length, size_t *length_read);
static int block_io_dev_write(
	io_entity_t *entity, const uintptr_t buffer, size_t length, size_t *length_written);

static const io_dev_funcs_t block_io_dev_dev_funcs = {
	.type		= block_io_dev_type,
	.open		= block_io_dev_open,
	.seek		= block_io_dev_seek,
	.size		= block_io_dev_size,
	.read		= block_io_dev_read,
	.write		= block_io_dev_write,
	.close		= block_io_dev_close,
	.dev_init	= NULL,
	.dev_close	= NULL
};


int block_io_dev_init(
	struct block_io_dev *this_instance,
	struct block_store *block_store,
	const struct uuid_octets *partition_guid,
	uintptr_t *dev_handle,
	uintptr_t *spec)
{
	this_instance->block_store = block_store;
	this_instance->partition_guid = *partition_guid;

	this_instance->file_pos = 0;
	this_instance->size = 0;
	this_instance->partition_handle = 0;

	this_instance->partition_info.block_size = 0;
	this_instance->partition_info.num_blocks = 0;

	this_instance->dev_info.funcs = &block_io_dev_dev_funcs;
	this_instance->dev_info.info = (uintptr_t)this_instance;

	*dev_handle = (uintptr_t)&this_instance->dev_info;
	*spec = (uintptr_t)this_instance;

	return 0;
}

void block_io_dev_deinit(
	struct block_io_dev *this_instance)
{
	(void)this_instance;
}

void block_io_dev_set_partition_guid(
	struct block_io_dev *this_instance,
	const struct uuid_octets *partition_guid)
{
	this_instance->partition_guid = *partition_guid;
}

static io_type_t block_io_dev_type(void)
{
	return IO_TYPE_BLOCK;
}

static int block_io_dev_open(
	io_dev_info_t *dev_info,
	const uintptr_t spec,
	io_entity_t *entity)
{
	struct block_io_dev *this_instance = (struct block_io_dev*)dev_info->info;
	psa_status_t psa_status = PSA_ERROR_BAD_STATE;

	psa_status = block_store_get_partition_info(this_instance->block_store,
		&this_instance->partition_guid,
		&this_instance->partition_info);

	if (psa_status == PSA_SUCCESS) {

		this_instance->file_pos = 0;
		this_instance->size =
			this_instance->partition_info.block_size * this_instance->partition_info.num_blocks;

		psa_status = block_store_open(this_instance->block_store, 0,
			&this_instance->partition_guid,
			&this_instance->partition_handle);

		entity->info = (uintptr_t)this_instance;
	}

	return (psa_status == PSA_SUCCESS) ? 0 : -EPERM;
}

static int block_io_dev_close(
	io_entity_t *entity)
{
	struct block_io_dev *this_instance = (struct block_io_dev*)entity->info;

	psa_status_t psa_status = block_store_close(this_instance->block_store, 0,
		this_instance->partition_handle);

	if (psa_status == PSA_SUCCESS) {

		this_instance->file_pos = 0;
		this_instance->size = 0;
	}

	return (psa_status == PSA_SUCCESS) ? 0 : -ENXIO;
}

static int block_io_dev_seek(
	io_entity_t *entity,
	int mode,
	signed long long offset)
{
	struct block_io_dev *this_instance = (struct block_io_dev*)entity->info;

	switch (mode)
	{
		case IO_SEEK_SET:
		{
			if (offset <= this_instance->size)
				this_instance->file_pos = (size_t)offset;
			else
				return -EINVAL;
			break;
		}
		case IO_SEEK_CUR:
		{
			ssize_t target_pos = this_instance->file_pos + offset;
			if ((target_pos >= 0) && (target_pos <= this_instance->size))
				this_instance->file_pos = (size_t)target_pos;
			else
				return -EINVAL;
			break;
		}
		default:
			return -EINVAL;
	}

	return 0;
}

static int block_io_dev_size(
	io_entity_t *entity,
	size_t *length)
{
	struct block_io_dev *this_instance = (struct block_io_dev*)entity->info;
	*length = this_instance->size;
	return 0;
}

static int block_io_dev_read(
	io_entity_t *entity,
	uintptr_t buffer,
	size_t length,
	size_t *length_read)
{
	struct block_io_dev *this_instance = (struct block_io_dev*)entity->info;
	size_t bytes_read = 0;
	*length_read = 0;

	if (!this_instance->partition_info.block_size)
		return -EIO;

	while ((bytes_read < length) && (this_instance->file_pos < this_instance->size)) {

		uint32_t lba = this_instance->file_pos / this_instance->partition_info.block_size;
		size_t offset = this_instance->file_pos % this_instance->partition_info.block_size;

		size_t bytes_remaining_in_block = this_instance->partition_info.block_size - offset;
		size_t bytes_remaining_in_file = this_instance->size - this_instance->file_pos;

		size_t bytes_remaining = length - bytes_read;
		if (bytes_remaining > bytes_remaining_in_file) bytes_remaining = bytes_remaining_in_file;

		size_t requested_len = (bytes_remaining < bytes_remaining_in_block) ?
			bytes_remaining : bytes_remaining_in_block;
		size_t actual_len = 0;

		psa_status_t psa_status = block_store_read(
			this_instance->block_store, 0,
			this_instance->partition_handle,
			lba, offset,
			requested_len,
			(uint8_t*)(buffer + bytes_read),
			&actual_len);

		if (psa_status != PSA_SUCCESS)
			return -EIO;

		bytes_read += actual_len;
		this_instance->file_pos += actual_len;
	}

	*length_read = bytes_read;
	return 0;
}

static int block_io_dev_write(
	io_entity_t *entity,
	const uintptr_t buffer,
	size_t length,
	size_t *length_written)
{
	struct block_io_dev *this_instance = (struct block_io_dev*)entity->info;
	size_t bytes_written = 0;
	*length_written = 0;

	if (!this_instance->partition_info.block_size)
		return -EIO;

	while ((bytes_written < length) && (this_instance->file_pos < this_instance->size)) {

		uint32_t lba = this_instance->file_pos / this_instance->partition_info.block_size;
		size_t offset = this_instance->file_pos % this_instance->partition_info.block_size;

		size_t bytes_remaining_in_block = this_instance->partition_info.block_size - offset;
		size_t bytes_remaining_in_file = this_instance->size - this_instance->file_pos;

		size_t bytes_remaining = length - bytes_written;
		if (bytes_remaining > bytes_remaining_in_file) bytes_remaining = bytes_remaining_in_file;

		size_t requested_len = (bytes_remaining < bytes_remaining_in_block) ?
			bytes_remaining : bytes_remaining_in_block;
		size_t actual_len = 0;

		psa_status_t psa_status = block_store_write(
			this_instance->block_store, 0,
			this_instance->partition_handle,
			lba, offset,
			(uint8_t*)(buffer + bytes_written),
			requested_len,
			&actual_len);

		if (psa_status != PSA_SUCCESS)
			return -EIO;

		bytes_written += actual_len;
		this_instance->file_pos += actual_len;
	}

	*length_written = bytes_written;
	return 0;
}

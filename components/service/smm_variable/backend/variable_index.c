/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdlib.h>
#include <string.h>
#include "variable_index.h"

/* Private functions */
static uint64_t name_hash(
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name)
{
	/* Using djb2 hash by Dan Bernstein */
	uint64_t hash = 5381;

	/* Calculate hash over GUID */
	hash = ((hash << 5) + hash) + guid->Data1;
	hash = ((hash << 5) + hash) + guid->Data2;
	hash = ((hash << 5) + hash) + guid->Data3;

	for (int i = 0; i < 8; ++i) {

		hash = ((hash << 5) + hash) + guid->Data4[i];
	}

	/* Extend to cover name up to but not including null terminator */
	for (int i = 0; i < name_size / sizeof(int16_t); ++i) {

		if (!name[i]) break;
		hash = ((hash << 5) + hash) + name[i];
	}

	return hash;
}

static uint64_t generate_uid(
	const struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name)
{
	uint64_t uid = name_hash(guid, name_size, name);

	/* todo - handle collision  */
	(void)context;

	return uid;
}

static int find_variable(
	const struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name)
{
	int found_pos = -1;
	uint64_t uid = name_hash(guid, name_size, name);

	for (int pos = 0; pos < context->max_variables; pos++) {

		if ((context->entries[pos].in_use) &&
			(uid == context->entries[pos].info.uid)) {

			found_pos = pos;
			break;
		}
	}

	return found_pos;
}

static int find_free(
	const struct variable_index *context)
{
	int free_pos = -1;

	for (int pos = 0; pos < context->max_variables; pos++) {

		if (!context->entries[pos].in_use) {

			free_pos = pos;
			break;
		}
	}

	return free_pos;
}

static void mark_dirty(struct variable_entry *entry)
{
	if (entry->info.attributes & EFI_VARIABLE_NON_VOLATILE)
		entry->dirty = true;
}

/* Public functions */
efi_status_t variable_index_init(
	struct variable_index *context,
	size_t max_variables)
{
	context->max_variables = max_variables;
	context->entries = (struct variable_entry*)
		malloc(sizeof(struct variable_entry) * max_variables);

	if (context->entries) {
		memset(context->entries, 0, sizeof(struct variable_entry) * max_variables);
	}

	return (context->entries) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}

void variable_index_deinit(
	struct variable_index *context)
{
	free(context->entries);
}

size_t variable_index_max_dump_size(
	struct variable_index *context)
{
	return sizeof(struct variable_info) * context->max_variables;
}

const struct variable_info *variable_index_find(
	const struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name)
{
	const struct variable_info *result = NULL;
	int pos = find_variable(context, guid, name_size, name);

	if (pos >= 0) {

		result = &context->entries[pos].info;
	}

	return result;
}

const struct variable_info *variable_index_find_next(
	const struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name)
{
	const struct variable_info *result = NULL;

	if (name_size >= sizeof(int16_t)) {

		/*
		 * Name must be at least one character long to accommodate
		 * the mandatory null terminator.
		 */
		if (name[0] != 0) {

			/* Find next from current name */
			int pos = find_variable(context, guid, name_size, name);

			if (pos >= 0) {

				/* Iterate to next used entry */
				++pos;
				while (pos < context->max_variables) {

					if (context->entries[pos].in_use) {

						result = &context->entries[pos].info;
						break;
					}

					++pos;
				}
			}
		}
		else {

			/* Find first */
			int pos = 0;

			while (pos < context->max_variables) {

				if (context->entries[pos].in_use) {

					result = &context->entries[pos].info;
					break;
				}

				++pos;
			}
		}
	}

	return result;
}

static void set_variable_name(
	struct variable_info *info,
	size_t name_size,
	const int16_t *name)
{
	size_t trimmed_size = 0;

	/* Trim the saved name to only include a single null terminator.
	 * Any additional terminators included in the client-set name size
	 * are discarded.
	 */
	for (size_t i = 0; i < name_size; i++) {

		++trimmed_size;
		info->name[i] = name[i];

		if (!name[i]) break;
	}

	info->name_size = trimmed_size * sizeof(int16_t);
}

const struct variable_info *variable_index_add(
	struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name,
	uint32_t attributes)
{
	struct variable_info *info = NULL;

	if (name_size <= (VARIABLE_INDEX_MAX_NAME_SIZE * sizeof(int16_t))) {

		int pos = find_free(context);

		if (pos >= 0) {

			struct variable_entry *entry = &context->entries[pos];

			info = &entry->info;
			info->uid = generate_uid(context, guid, name_size, name);
			info->guid = *guid;
			info->attributes = attributes;
			set_variable_name(info, name_size, name);

			mark_dirty(entry);
			entry->in_use = true;
		}
	}

	return info;
}

void variable_index_remove(
	struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name)
{
	int pos = find_variable(context, guid, name_size, name);

	if (pos >= 0) {

		struct variable_entry *entry = &context->entries[pos];
		mark_dirty(entry);
		entry->in_use = false;

		memset(&entry->info, 0, sizeof(struct variable_info));
	}
}

void variable_index_update_attributes(
	struct variable_index *context,
	const struct variable_info *info,
	uint32_t attributes)
{
	if (info) {

		struct variable_info *modified_info = (struct variable_info*)info;

		size_t info_offset = offsetof(struct variable_entry, info);
		struct variable_entry *entry = (struct variable_entry*)((uint8_t*)modified_info - info_offset);

		if (attributes != modified_info->attributes) {

			modified_info->attributes = attributes;
			mark_dirty(entry);
		}
	}
}

bool variable_index_dump(
	struct variable_index *context,
	size_t buffer_size,
	uint8_t *buffer,
	size_t *data_len)
{
	bool any_dirty = false;
	uint8_t *dump_pos = buffer;
	size_t bytes_dumped = 0;

	for (int pos = 0; pos < context->max_variables; pos++) {

		struct variable_entry *entry = &context->entries[pos];
		struct variable_info *info = &entry->info;

		if (entry->in_use &&
			(info->attributes & EFI_VARIABLE_NON_VOLATILE) &&
			((bytes_dumped + sizeof(struct variable_info)) <= buffer_size)) {

			memcpy(dump_pos, info, sizeof(struct variable_info));
			bytes_dumped += sizeof(struct variable_info);
			dump_pos += sizeof(struct variable_info);
		}

		any_dirty |= entry->dirty;
		entry->dirty = false;
	}

	*data_len = bytes_dumped;

	return any_dirty;
}

size_t variable_index_restore(
	const struct variable_index *context,
	size_t data_len,
	const uint8_t *buffer)
{
	size_t bytes_loaded = 0;
	const uint8_t *load_pos = buffer;
	int pos = 0;

	while (bytes_loaded < data_len) {

		if ((data_len - bytes_loaded) >= sizeof(struct variable_info)) {

			struct variable_entry *entry = &context->entries[pos];
			struct variable_info *info = &entry->info;

			memcpy(info, load_pos, sizeof(struct variable_info));

			entry->in_use = true;

			bytes_loaded += sizeof(struct variable_info);
			load_pos += sizeof(struct variable_info);

			++pos;
		}
		else {

			/* Not a whole number of variable_info structs! */
			break;
		}
	}

	return bytes_loaded;
}

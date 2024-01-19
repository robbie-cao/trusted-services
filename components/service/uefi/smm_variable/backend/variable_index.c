/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "variable_index.h"

#include <stdlib.h>
#include <string.h>

/* Private functions */
static uint64_t name_hash(const EFI_GUID *guid, size_t name_size, const int16_t *name)
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
	for (size_t i = 0; i < (name_size - sizeof(int16_t)) / sizeof(int16_t); ++i) {
		hash = ((hash << 5) + hash) + name[i];
	}

	return hash;
}

static uint64_t generate_uid(const struct variable_index *context, const EFI_GUID *guid,
			     size_t name_size, const int16_t *name)
{
	uint64_t uid = name_hash(guid, name_size, name);

	/* todo - handle collision  */
	(void)context;

	return uid;
}

static int find_variable(const struct variable_index *context, const EFI_GUID *guid,
			 size_t name_size, const int16_t *name)
{
	int found_pos = -1;
	uint64_t uid = name_hash(guid, name_size, name);

	for (size_t pos = 0; pos < context->max_variables; pos++) {
		if ((context->entries[pos].in_use) &&
		    (uid == context->entries[pos].info.metadata.uid)) {
			found_pos = pos;
			break;
		}
	}

	return found_pos;
}

static int find_free(const struct variable_index *context)
{
	int free_pos = -1;

	for (size_t pos = 0; pos < context->max_variables; pos++) {
		if (!context->entries[pos].in_use) {
			free_pos = pos;
			break;
		}
	}

	return free_pos;
}

static void mark_dirty(struct variable_entry *entry)
{
	if (entry->info.metadata.attributes & EFI_VARIABLE_NON_VOLATILE)
		entry->dirty = true;
}

static struct variable_entry *containing_entry(const struct variable_info *info)
{
	size_t info_offset = offsetof(struct variable_entry, info);
	struct variable_entry *entry = (struct variable_entry *)((uint8_t *)info - info_offset);
	return entry;
}

/* Public functions */
efi_status_t variable_index_init(struct variable_index *context, size_t max_variables)
{
	context->max_variables = max_variables;
	context->entries =
		(struct variable_entry *)malloc(sizeof(struct variable_entry) * max_variables);

	if (context->entries) {
		memset(context->entries, 0, sizeof(struct variable_entry) * max_variables);
	}

	return (context->entries) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}

void variable_index_deinit(struct variable_index *context)
{
	free(context->entries);
}

size_t variable_index_max_dump_size(struct variable_index *context)
{
	return sizeof(struct variable_metadata) * context->max_variables;
}

struct variable_info *variable_index_find(const struct variable_index *context,
					  const EFI_GUID *guid, size_t name_size,
					  const int16_t *name)
{
	struct variable_info *result = NULL;
	int pos = find_variable(context, guid, name_size, name);

	if (pos >= 0) {
		result = &context->entries[pos].info;
	}

	return result;
}

struct variable_info *variable_index_find_next(const struct variable_index *context,
					       const EFI_GUID *guid, size_t name_size,
					       const int16_t *name, efi_status_t *status)
{
	struct variable_info *result = NULL;
	*status = EFI_NOT_FOUND;

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
				while (pos < (int)context->max_variables) {
					if (context->entries[pos].in_use &&
					    context->entries[pos].info.is_variable_set) {
						result = &context->entries[pos].info;
						*status = EFI_SUCCESS;
						break;
					}

					++pos;
				}
			} else {
				/* A non-empty name was provided but it wasn't found */
				*status = EFI_INVALID_PARAMETER;
			}
		} else {
			/* Find first */
			int pos = 0;

			while (pos < (int)context->max_variables) {
				if (context->entries[pos].in_use &&
				    context->entries[pos].info.is_variable_set) {
					result = &context->entries[pos].info;
					*status = EFI_SUCCESS;
					break;
				}

				++pos;
			}
		}
	}

	return result;
}

static struct variable_entry *add_entry(const struct variable_index *context, const EFI_GUID *guid,
					size_t name_size, const int16_t *name)
{
	struct variable_entry *entry = NULL;

	if (name_size <= (VARIABLE_INDEX_MAX_NAME_SIZE * sizeof(int16_t))) {
		int pos = find_free(context);

		if (pos >= 0) {
			entry = &context->entries[pos];

			struct variable_info *info = &entry->info;

			/* Initialize metadata */
			info->metadata.uid = generate_uid(context, guid, name_size, name);
			info->metadata.guid = *guid;
			info->metadata.attributes = 0;
			info->metadata.name_size = name_size;
			memcpy(info->metadata.name, name, name_size);

			info->is_constraints_set = false;
			info->is_variable_set = false;

			entry->in_use = true;
		}
	}

	return entry;
}

struct variable_info *variable_index_add_entry(const struct variable_index *context,
					       const EFI_GUID *guid, size_t name_size,
					       const int16_t *name)
{
	struct variable_info *info = NULL;
	struct variable_entry *entry = add_entry(context, guid, name_size, name);

	if (entry) {
		info = &entry->info;
	}

	return info;
}

void variable_index_remove_unused_entry(const struct variable_index *context,
					struct variable_info *info)
{
	(void)context;

	if (info && !info->is_constraints_set && !info->is_variable_set) {
		struct variable_entry *entry = containing_entry(info);
		entry->in_use = false;

		memset(info, 0, sizeof(struct variable_info));
	}
}

void variable_index_set_variable(struct variable_info *info, uint32_t attributes)
{
	struct variable_entry *entry = containing_entry(info);

	info->metadata.attributes = attributes;
	info->is_variable_set = true;

	mark_dirty(entry);
}

void variable_index_clear_variable(const struct variable_index *context, struct variable_info *info)
{
	(void)context;

	if (info) {
		struct variable_entry *entry = containing_entry(info);
		mark_dirty(entry);

		/* Mark variable as no longer set */
		entry->info.is_variable_set = false;
	}
}

void variable_index_set_constraints(struct variable_info *info,
				    const struct variable_constraints *constraints)
{
	if (info) {
		info->check_constraints = *constraints;
		info->is_constraints_set = true;
	}
}

bool variable_index_dump(const struct variable_index *context, size_t buffer_size, uint8_t *buffer,
			 size_t *data_len)
{
	bool any_dirty = false;
	uint8_t *dump_pos = buffer;
	size_t bytes_dumped = 0;

	for (size_t pos = 0; pos < context->max_variables; pos++) {
		struct variable_entry *entry = &context->entries[pos];
		struct variable_metadata *metadata = &entry->info.metadata;

		if (entry->in_use && entry->info.is_variable_set &&
		    (metadata->attributes & EFI_VARIABLE_NON_VOLATILE) &&
		    ((bytes_dumped + sizeof(struct variable_metadata)) <= buffer_size)) {
			memcpy(dump_pos, metadata, sizeof(struct variable_metadata));
			bytes_dumped += sizeof(struct variable_metadata);
			dump_pos += sizeof(struct variable_metadata);
		}

		any_dirty |= entry->dirty;
		entry->dirty = false;
	}

	*data_len = bytes_dumped;

	return any_dirty;
}

size_t variable_index_restore(const struct variable_index *context, size_t data_len,
			      const uint8_t *buffer)
{
	size_t bytes_loaded = 0;
	const uint8_t *load_pos = buffer;
	int pos = 0;

	while (bytes_loaded < data_len) {
		if ((data_len - bytes_loaded) >= sizeof(struct variable_metadata)) {
			struct variable_entry *entry = &context->entries[pos];
			struct variable_metadata *metadata = &entry->info.metadata;

			memcpy(metadata, load_pos, sizeof(struct variable_metadata));

			entry->info.is_variable_set = true;
			entry->in_use = true;

			bytes_loaded += sizeof(struct variable_metadata);
			load_pos += sizeof(struct variable_metadata);

			++pos;
		} else {
			/* Not a whole number of variable_metadata structs! */
			break;
		}
	}

	return bytes_loaded;
}

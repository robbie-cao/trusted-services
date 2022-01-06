/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "uefi_variable_store.h"
#include "variable_index_iterator.h"
#include "variable_checker.h"

/* Private functions */
static void load_variable_index(
	struct uefi_variable_store *context);

static efi_status_t sync_variable_index(
	struct uefi_variable_store *context);

static efi_status_t check_capabilities(
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var);

static efi_status_t check_access_permitted(
	const struct uefi_variable_store *context,
	const struct variable_info *info);

static efi_status_t check_access_permitted_on_set(
	const struct uefi_variable_store *context,
	const struct variable_info *info,
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var);

static efi_status_t store_variable_data(
	struct uefi_variable_store *context,
	const struct variable_info *info,
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var);

static efi_status_t remove_variable_data(
	struct uefi_variable_store *context,
	const struct variable_info *info);

static efi_status_t load_variable_data(
	struct uefi_variable_store *context,
	const struct variable_info *info,
	SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var,
	size_t max_data_len);

static psa_status_t store_overwrite(
	struct delegate_variable_store *delegate_store,
	uint32_t client_id,
	uint64_t uid,
	size_t data_length,
	const void *data);

static psa_status_t store_append_write(
	struct delegate_variable_store *delegate_store,
	uint32_t client_id,
	uint64_t uid,
	size_t data_length,
	const void *data);

static void purge_orphan_index_entries(
	struct uefi_variable_store *context);

static struct delegate_variable_store *select_delegate_store(
	struct uefi_variable_store *context,
	uint32_t attributes);

static size_t space_used(
	struct uefi_variable_store *context,
	uint32_t attributes,
	struct storage_backend *storage_backend);

static efi_status_t psa_to_efi_storage_status(
	psa_status_t psa_status);

static efi_status_t check_name_terminator(
	const int16_t *name,
	size_t name_size);

/* Private UID for storing the variable index - may be overridden at build-time */
#ifndef SMM_VARIABLE_INDEX_STORAGE_UID
#define SMM_VARIABLE_INDEX_STORAGE_UID			(1)
#endif

/* Default maximum variable size -
 * may be overridden using uefi_variable_store_set_storage_limits()
 */
#define DEFAULT_MAX_VARIABLE_SIZE			(2048)

efi_status_t uefi_variable_store_init(
	struct uefi_variable_store *context,
	uint32_t owner_id,
	size_t max_variables,
	struct storage_backend *persistent_store,
	struct storage_backend *volatile_store)
{
	efi_status_t status = EFI_SUCCESS;

	/* Initialise persistent store defaults */
	context->persistent_store.is_nv = true;
	context->persistent_store.max_variable_size = DEFAULT_MAX_VARIABLE_SIZE;
	context->persistent_store.total_capacity = DEFAULT_MAX_VARIABLE_SIZE * max_variables;
	context->persistent_store.storage_backend = persistent_store;

	/* Initialise volatile store defaults */
	context->volatile_store.is_nv = false;
	context->volatile_store.max_variable_size = DEFAULT_MAX_VARIABLE_SIZE;
	context->volatile_store.total_capacity = DEFAULT_MAX_VARIABLE_SIZE * max_variables;
	context->volatile_store.storage_backend = volatile_store;

	context->owner_id = owner_id;
	context->is_boot_service = true;

	status = variable_index_init(&context->variable_index, max_variables);

	if (status == EFI_SUCCESS) {

		/* Allocate a buffer for synchronizing the variable index with the persistent store */
		context->index_sync_buffer_size = variable_index_max_dump_size(&context->variable_index);
		context->index_sync_buffer = NULL;

		if (context->index_sync_buffer_size) {

			context->index_sync_buffer = malloc(context->index_sync_buffer_size);
			status = (context->index_sync_buffer) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
		}

		/* Load the variable index with NV variable info from the persistent store */
		if (context->index_sync_buffer) {

			load_variable_index(context);
			purge_orphan_index_entries(context);
		}
	}

	return status;
}

void uefi_variable_store_deinit(
	struct uefi_variable_store *context)
{
	variable_index_deinit(&context->variable_index);

	free(context->index_sync_buffer);
	context->index_sync_buffer = NULL;
}

void uefi_variable_store_set_storage_limits(
	struct uefi_variable_store *context,
	uint32_t attributes,
	size_t total_capacity,
	size_t max_variable_size)
{
	struct delegate_variable_store *delegate_store = select_delegate_store(
		context,
		attributes);

	delegate_store->total_capacity = total_capacity;
	delegate_store->max_variable_size = max_variable_size;
}

efi_status_t uefi_variable_store_set_variable(
	struct uefi_variable_store *context,
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var)
{
	bool should_sync_index = false;

	/* Validate incoming request */
	efi_status_t status = check_name_terminator(var->Name, var->NameSize);
	if (status != EFI_SUCCESS) return status;

	status = check_capabilities(var);
	if (status != EFI_SUCCESS) return status;

	/* Find an existing entry in the variable index or add a new one */
	struct variable_info *info = variable_index_find(
		&context->variable_index,
		&var->Guid,
		var->NameSize,
		var->Name);

	if (!info) {

		info = variable_index_add_entry(
			&context->variable_index,
			&var->Guid,
			var->NameSize,
			var->Name);

		if (!info) return EFI_OUT_OF_RESOURCES;
	}

	/* Control access */
	status = check_access_permitted_on_set(context, info, var);

	if (status == EFI_SUCCESS) {

		/* Access permitted */
		if (info->is_variable_set) {

			/* It's a request to update to an existing variable */
			if (!(var->Attributes &
				(EFI_VARIABLE_APPEND_WRITE | EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS_MASK)) &&
				!var->DataSize) {

				/* It's a remove operation - for a remove, the variable
				 * data must be removed from the storage backend before
				 * modifying and syncing the variable index.  This ensures
				 * that it's never possible for an object to exist within
				 * the storage backend without a corresponding index entry.
				 */
				remove_variable_data(context, info);
				variable_index_clear_variable(&context->variable_index, info);

				should_sync_index = (var->Attributes & EFI_VARIABLE_NON_VOLATILE);
			}
			else {

				/* It's a set operation where variable data is potentially
				 * being overwritten or extended.
				 */
				if ((var->Attributes & ~EFI_VARIABLE_APPEND_WRITE) != info->metadata.attributes) {

					/* Modifying attributes is forbidden */
					return EFI_INVALID_PARAMETER;
				}
			}
		}
		else {

			if (var->DataSize) {

				/*  It's a request to create a new variable */
				variable_index_set_variable(info, var->Attributes);
				should_sync_index = (var->Attributes & EFI_VARIABLE_NON_VOLATILE);
			}
			else {

				/* Attempting to remove a non-existent variable */
				return EFI_NOT_FOUND;
			}
		}
	}

	/* The order of these operations is important. For an update
	 * or create operation, The variable index is always synchronized
	 * to NV storage first, then the variable data is stored. If the
	 * data store operation fails or doesn't happen due to a power failure,
	 * the inconistency between the variable index and the persistent
	 * store is detectable and may be corrected by purging the corresponding
	 * index entry.
	 */
	if (should_sync_index) {

		status = sync_variable_index(context);
	}

	/* Store any variable data to the storage backend */
	if (info->is_variable_set && (status == EFI_SUCCESS)) {

		status = store_variable_data(context, info, var);
	}

	variable_index_remove_unused_entry(&context->variable_index, info);

	return status;
}

efi_status_t uefi_variable_store_get_variable(
	struct uefi_variable_store *context,
	SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var,
	size_t max_data_len,
	size_t *total_length)
{
	efi_status_t status = check_name_terminator(var->Name, var->NameSize);
	if (status != EFI_SUCCESS) return status;

	status = EFI_NOT_FOUND;
	*total_length = 0;

	const struct variable_info *info = variable_index_find(
		&context->variable_index,
		&var->Guid,
		var->NameSize,
		var->Name);

	if (info && info->is_variable_set) {

		/* Variable already exists */
		status = check_access_permitted(context, info);

		if (status == EFI_SUCCESS) {

			status = load_variable_data(context, info, var, max_data_len);
			var->Attributes = info->metadata.attributes;
			*total_length = SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_TOTAL_SIZE(var);
		}
	}

	return status;
}

efi_status_t uefi_variable_store_get_next_variable_name(
	struct uefi_variable_store *context,
	SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME *cur,
	size_t max_name_len,
	size_t *total_length)
{
	efi_status_t status = check_name_terminator(cur->Name, cur->NameSize);
	if (status != EFI_SUCCESS) return status;

	*total_length = 0;

	const struct variable_info *info = variable_index_find_next(
		&context->variable_index,
		&cur->Guid,
		cur->NameSize,
		cur->Name,
		&status);

	if (info && (status == EFI_SUCCESS)) {

		if (info->metadata.name_size <= max_name_len) {

			cur->Guid = info->metadata.guid;
			cur->NameSize = info->metadata.name_size;
			memcpy(cur->Name, info->metadata.name, info->metadata.name_size);

			*total_length = SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_TOTAL_SIZE(cur);
		}
		else {

			status = EFI_BUFFER_TOO_SMALL;
		}
	}

	return status;
}

efi_status_t uefi_variable_store_query_variable_info(
	struct uefi_variable_store *context,
	SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO *var_info)
{
	struct delegate_variable_store *delegate_store = select_delegate_store(
		context,
		var_info->Attributes);

	size_t total_used = space_used(
		context,
		var_info->Attributes,
		delegate_store->storage_backend);

	var_info->MaximumVariableSize = delegate_store->max_variable_size;
	var_info->MaximumVariableStorageSize = delegate_store->total_capacity;
	var_info->RemainingVariableStorageSize = (total_used < delegate_store->total_capacity) ?
		delegate_store->total_capacity - total_used :
		0;

	return EFI_SUCCESS;
}

efi_status_t uefi_variable_store_exit_boot_service(
	struct uefi_variable_store *context)
{
	context->is_boot_service = false;
	return EFI_SUCCESS;
}

efi_status_t uefi_variable_store_set_var_check_property(
	struct uefi_variable_store *context,
	const SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *property)
{
	efi_status_t status = check_name_terminator(property->Name, property->NameSize);
	if (status != EFI_SUCCESS) return status;

	/* Find in index or create a new entry */
	struct variable_info *info = variable_index_find(
		&context->variable_index,
		&property->Guid,
		property->NameSize,
		property->Name);

	if (!info) {

		info = variable_index_add_entry(
			&context->variable_index,
			&property->Guid,
			property->NameSize,
			property->Name);

		if (!info) return EFI_OUT_OF_RESOURCES;
	}

	/* Applying check constraints to an existing variable that may have
	 * constraints already set.  These could constrain the setting of
	 * the constraints.
	 */
	struct variable_constraints constraints = info->check_constraints;

	status = variable_checker_set_constraints(
		&constraints,
		info->is_constraints_set,
		&property->VariableProperty);

	if (status == EFI_SUCCESS) {

		variable_index_set_constraints(info, &constraints);
	}

	variable_index_remove_unused_entry(&context->variable_index, info);

	return status;
}

efi_status_t uefi_variable_store_get_var_check_property(
	struct uefi_variable_store *context,
	SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *property)
{
	efi_status_t status = check_name_terminator(property->Name, property->NameSize);
	if (status != EFI_SUCCESS) return status;

	status = EFI_NOT_FOUND;

	const struct variable_info *info = variable_index_find(
		&context->variable_index,
		&property->Guid,
		property->NameSize,
		property->Name);

	if (info && info->is_constraints_set) {

		variable_checker_get_constraints(
			&info->check_constraints,
			&property->VariableProperty);

		status = EFI_SUCCESS;
	}

	return status;
}

static void load_variable_index(
	struct uefi_variable_store *context)
{
	struct storage_backend *persistent_store = context->persistent_store.storage_backend;

	if (persistent_store) {

		size_t data_len = 0;

		psa_status_t psa_status = persistent_store->interface->get(
			persistent_store->context,
			context->owner_id,
			SMM_VARIABLE_INDEX_STORAGE_UID,
			0,
			context->index_sync_buffer_size,
			context->index_sync_buffer,
			&data_len);

		if (psa_status == PSA_SUCCESS) {

			variable_index_restore(&context->variable_index, data_len, context->index_sync_buffer);
		}
	}
}

static efi_status_t sync_variable_index(
	struct uefi_variable_store *context)
{
	efi_status_t status = EFI_SUCCESS;

	/* Sync the varibale index to storage if anything is dirty */
	size_t data_len = 0;

	bool is_dirty = variable_index_dump(
		&context->variable_index,
		context->index_sync_buffer_size,
		context->index_sync_buffer,
		&data_len);

	if (is_dirty) {

		struct storage_backend *persistent_store = context->persistent_store.storage_backend;

		if (persistent_store) {

			psa_status_t psa_status = persistent_store->interface->set(
				persistent_store->context,
				context->owner_id,
				SMM_VARIABLE_INDEX_STORAGE_UID,
				data_len,
				context->index_sync_buffer,
				PSA_STORAGE_FLAG_NONE);

			status = psa_to_efi_storage_status(psa_status);
		}
	}

	return status;
}

static efi_status_t check_capabilities(
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var)
{
	/* Check if invalid variable attributes have been requested */
	if ((var->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) &&
		!(var->Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)) {

		/*
		 * Client is required to explicitly allow bootservice access for runtime
		 * access variables.
		 */
		return EFI_INVALID_PARAMETER;
	}

	/* Check if any unsupported variable attributes have been requested */
	if (var->Attributes & ~(
		EFI_VARIABLE_NON_VOLATILE |
		EFI_VARIABLE_BOOTSERVICE_ACCESS |
		EFI_VARIABLE_RUNTIME_ACCESS |
		EFI_VARIABLE_APPEND_WRITE)) {

		/* An unsupported attribute has been requested */
		return EFI_UNSUPPORTED;
	}

	return EFI_SUCCESS;
}

static efi_status_t check_access_permitted(
	const struct uefi_variable_store *context,
	const struct variable_info *info)
{
	efi_status_t status = EFI_SUCCESS;

	if (info->is_variable_set &&
		(info->metadata.attributes &
			(EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS))) {

		/* Access is controlled */
		status = EFI_ACCESS_DENIED;

		if (context->is_boot_service) {

			if (info->metadata.attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)
				status = EFI_SUCCESS;
		}
		else {

			if (info->metadata.attributes & EFI_VARIABLE_RUNTIME_ACCESS)
				status = EFI_SUCCESS;
		}
	}

	return status;
}

static efi_status_t check_access_permitted_on_set(
	const struct uefi_variable_store *context,
	const struct variable_info *info,
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var)
{
	efi_status_t status = check_access_permitted(context, info);

	if ((status == EFI_SUCCESS) && info->is_constraints_set) {

		/* Apply check constraints */
		status = variable_checker_check_on_set(
			&info->check_constraints,
			var->Attributes,
			var->DataSize);
	}

	return status;
}

static efi_status_t store_variable_data(
	struct uefi_variable_store *context,
	const struct variable_info *info,
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var)
{
	psa_status_t psa_status = PSA_SUCCESS;
	size_t data_len = var->DataSize;
	const uint8_t *data = (const uint8_t*)var +
		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(var);

	struct delegate_variable_store *delegate_store = select_delegate_store(
		context,
		info->metadata.attributes);

	if (delegate_store->storage_backend) {

		if (!(var->Attributes & EFI_VARIABLE_APPEND_WRITE)) {

			/* Create or overwrite variable data */
			psa_status = store_overwrite(
				delegate_store,
				context->owner_id,
				info->metadata.uid,
				data_len,
				data);
		}
		else {

			/* Append new data to existing variable data */
			psa_status = store_append_write(
				delegate_store,
				context->owner_id,
				info->metadata.uid,
				data_len,
				data);
		}
	}

	if ((psa_status != PSA_SUCCESS) && delegate_store->is_nv) {

		/* A storage failure has occurred so attempt to fix any
		 * mismatch between the variable index and stored NV variables.
		 */
		purge_orphan_index_entries(context);
	}

	return psa_to_efi_storage_status(psa_status);
}

static efi_status_t remove_variable_data(
	struct uefi_variable_store *context,
	const struct variable_info *info)
{
	psa_status_t psa_status = PSA_SUCCESS;

	if (info->is_variable_set) {

		struct delegate_variable_store *delegate_store = select_delegate_store(
			context,
			info->metadata.attributes);

		if (delegate_store->storage_backend) {

			psa_status = delegate_store->storage_backend->interface->remove(
				delegate_store->storage_backend->context,
				context->owner_id,
				info->metadata.uid);
		}
	}

	return psa_to_efi_storage_status(psa_status);
}

static efi_status_t load_variable_data(
	struct uefi_variable_store *context,
	const struct variable_info *info,
	SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var,
	size_t max_data_len)
{
	psa_status_t psa_status = PSA_SUCCESS;
	size_t data_len = 0;
	uint8_t *data = (uint8_t*)var +
		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(var);

	struct delegate_variable_store *delegate_store = select_delegate_store(
		context,
		info->metadata.attributes);

	if (delegate_store->storage_backend) {

		psa_status = delegate_store->storage_backend->interface->get(
			delegate_store->storage_backend->context,
			context->owner_id,
			info->metadata.uid,
			0,
			max_data_len,
			data,
			&data_len);

		var->DataSize = data_len;
	}

	return psa_to_efi_storage_status(psa_status);
}

static psa_status_t store_overwrite(
	struct delegate_variable_store *delegate_store,
	uint32_t client_id,
	uint64_t uid,
	size_t data_length,
	const void *data)
{
	/* Police maximum variable size limit */
	if (data_length > delegate_store->max_variable_size) return PSA_ERROR_INVALID_ARGUMENT;

	psa_status_t psa_status = delegate_store->storage_backend->interface->set(
		delegate_store->storage_backend->context,
		client_id,
		uid,
		data_length,
		data,
		PSA_STORAGE_FLAG_NONE);

	return psa_status;
}

static psa_status_t store_append_write(
	struct delegate_variable_store *delegate_store,
	uint32_t client_id,
	uint64_t uid,
	size_t data_length,
	const void *data)
{
	struct psa_storage_info_t storage_info;

	if (data_length == 0) return PSA_SUCCESS;

	psa_status_t psa_status = delegate_store->storage_backend->interface->get_info(
		delegate_store->storage_backend->context,
		client_id,
		uid,
		&storage_info);

	if (psa_status != PSA_SUCCESS) return psa_status;

	/* Determine size of appended variable */
	size_t new_size = storage_info.size + data_length;

	/* Defend against integer overflow */
	if (new_size < storage_info.size) return PSA_ERROR_INVALID_ARGUMENT;

		/* Police maximum variable size limit */
	if (new_size > delegate_store->max_variable_size) return PSA_ERROR_INVALID_ARGUMENT;

	/* Storage backend doesn't support an append operation so we need
	 * need to read the current variable data, extend it and write it back.
	 */
	uint8_t *rw_buf = malloc(new_size);
	if (!rw_buf) return PSA_ERROR_INSUFFICIENT_MEMORY;

	size_t old_size = 0;
	psa_status = delegate_store->storage_backend->interface->get(
		delegate_store->storage_backend->context,
		client_id,
		uid,
		0,
		new_size,
		rw_buf,
		&old_size);

	if (psa_status == PSA_SUCCESS) {

		if ((old_size + data_length) <= new_size) {

			/* Extend the variable data */
			memcpy(&rw_buf[old_size], data, data_length);

			psa_status = delegate_store->storage_backend->interface->set(
				delegate_store->storage_backend->context,
				client_id,
				uid,
				old_size + data_length,
				rw_buf,
				storage_info.flags);
		}
		else {

			/* There's a mismatch between the length obtained from
			 * get_info() and the subsequent length returned by get().
			 */
			psa_status = PSA_ERROR_STORAGE_FAILURE;
		}
	}

	free(rw_buf);

	return psa_status;
}

static void purge_orphan_index_entries(
	struct uefi_variable_store *context)
{
	bool any_orphans = false;
	struct variable_index_iterator iter;
	variable_index_iterator_first(&iter, &context->variable_index);

	/* Iterate over variable index looking for any entries for NV
	 * variables where there is no corresponding object in the
	 * persistent store. This condition could arise due to
	 * a power failure before an object is stored.
	 */
	while (!variable_index_iterator_is_done(&iter)) {

		struct variable_info *info = variable_index_iterator_current(&iter);

		if (info->is_variable_set && (info->metadata.attributes & EFI_VARIABLE_NON_VOLATILE)) {

			struct psa_storage_info_t storage_info;
			struct storage_backend *storage_backend = context->persistent_store.storage_backend;

			psa_status_t psa_status = storage_backend->interface->get_info(
				storage_backend->context,
				context->owner_id,
				info->metadata.uid,
				&storage_info);

			if (psa_status != PSA_SUCCESS) {

				/* Detected a mismatch between the index and storage */
				variable_index_clear_variable(&context->variable_index, info);
				any_orphans = true;
			}
		}

		variable_index_iterator_next(&iter);
	}

	if (any_orphans) sync_variable_index(context);
}

static struct delegate_variable_store *select_delegate_store(
	struct uefi_variable_store *context,
	uint32_t attributes)
{
	bool is_nv = (attributes & EFI_VARIABLE_NON_VOLATILE);

	return (is_nv) ?
		&context->persistent_store :
		&context->volatile_store;
}

static size_t space_used(
	struct uefi_variable_store *context,
	uint32_t attributes,
	struct storage_backend *storage_backend)
{
	if (!storage_backend) return 0;

	size_t total_used = 0;
	struct variable_index_iterator iter;
	variable_index_iterator_first(&iter, &context->variable_index);

	while (!variable_index_iterator_is_done(&iter)) {

		struct variable_info *info = variable_index_iterator_current(&iter);

		if (info->is_variable_set &&
		    ((info->metadata.attributes & EFI_VARIABLE_NON_VOLATILE) ==
			 (attributes & EFI_VARIABLE_NON_VOLATILE))) {

			struct psa_storage_info_t storage_info;

			psa_status_t psa_status = storage_backend->interface->get_info(
				storage_backend->context,
				context->owner_id,
				info->metadata.uid,
				&storage_info);

			if (psa_status == PSA_SUCCESS) total_used += storage_info.size;
		}

		variable_index_iterator_next(&iter);
	}

	return total_used;
}

static efi_status_t psa_to_efi_storage_status(
	psa_status_t psa_status)
{
	efi_status_t efi_status = EFI_DEVICE_ERROR;

	switch(psa_status)
	{
		case PSA_SUCCESS:
			efi_status = EFI_SUCCESS;
			break;
		case PSA_ERROR_NOT_PERMITTED:
			efi_status = EFI_ACCESS_DENIED;
			break;
		case PSA_ERROR_INVALID_ARGUMENT:
			efi_status = EFI_INVALID_PARAMETER;
			break;
		case PSA_ERROR_BAD_STATE:
			efi_status = EFI_NOT_READY;
			break;
		case PSA_ERROR_BUFFER_TOO_SMALL:
			efi_status = EFI_BUFFER_TOO_SMALL;
			break;
		case PSA_ERROR_DOES_NOT_EXIST:
			efi_status = EFI_NOT_FOUND;
			break;
		case PSA_ERROR_INSUFFICIENT_MEMORY:
			efi_status = EFI_OUT_OF_RESOURCES;
			break;
		case PSA_ERROR_INSUFFICIENT_STORAGE:
			efi_status = EFI_OUT_OF_RESOURCES;
			break;
		case PSA_ERROR_STORAGE_FAILURE:
			efi_status = EFI_DEVICE_ERROR;
			break;
		case PSA_STATUS_HARDWARE_FAILURE:
			efi_status = EFI_DEVICE_ERROR;
			break;
		default:
			break;
	}

	return efi_status;
}

static efi_status_t check_name_terminator(
	const int16_t *name,
	size_t name_size)
{
	/* Variable names must be null terminated */
	if (name_size < sizeof(int16_t) || name[name_size/sizeof (int16_t) - 1] != L'\0') {

		return EFI_INVALID_PARAMETER;
	}

	return EFI_SUCCESS;
}

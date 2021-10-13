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

/* Private functions */
static void load_variable_index(
	struct uefi_variable_store *context);

static efi_status_t sync_variable_index(
	struct uefi_variable_store *context);

static efi_status_t check_access_permitted(
	const struct uefi_variable_store *context,
	const struct variable_info *info);

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

static void purge_orphan_index_entries(
	struct uefi_variable_store *context);

static efi_status_t psa_to_efi_storage_status(
	psa_status_t psa_status);

static efi_status_t check_name_terminator(
	const int16_t *name,
	size_t name_size);

/* Private UID for storing the variable index */
#define VARIABLE_INDEX_STORAGE_UID			(1)


efi_status_t uefi_variable_store_init(
	struct uefi_variable_store *context,
	uint32_t owner_id,
	size_t max_variables,
	struct storage_backend *persistent_store,
	struct storage_backend *volatile_store)
{
	efi_status_t status = EFI_SUCCESS;

	context->persistent_store = persistent_store;
	context->volatile_store = volatile_store;

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

efi_status_t uefi_variable_store_set_variable(
	struct uefi_variable_store *context,
	const SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *var)
{
	efi_status_t status = check_name_terminator(var->Name, var->NameSize);
	if (status != EFI_SUCCESS) return status;

	bool should_sync_index = false;

	/* Find in index */
	const struct variable_info *info = variable_index_find(
		&context->variable_index,
		&var->Guid,
		var->NameSize,
		var->Name);

	if (info) {

		/* Variable already exists */
		status = check_access_permitted(context, info);

		if (status == EFI_SUCCESS) {

			should_sync_index = (info->attributes & EFI_VARIABLE_NON_VOLATILE);

			if (var->DataSize) {

				/* It's an update to an existing variable */
				variable_index_update_attributes(
					&context->variable_index,
					info,
					var->Attributes);
			}
			else {

				/* It's a remove operation - for a remove, the variable
				 * data must be removed from the storage backend before
				 * modifying and syncing the variable index.  This ensures
				 * that it's never possible for an object to exist within
				 * the storage backend without a corresponding index entry.
				 */
				remove_variable_data(context, info);
				variable_index_remove(&context->variable_index, info);

				/* Variable info no longer valid */
				info = NULL;
			}
		}
		else {

			/* Access forbidden */
			info = NULL;
		}
	}
	else if (var->DataSize) {

		/* It's a new variable */
		info = variable_index_add(
			&context->variable_index,
			&var->Guid,
			var->NameSize,
			var->Name,
			var->Attributes);

		should_sync_index = info && (info->attributes & EFI_VARIABLE_NON_VOLATILE);
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
	if (info && (status == EFI_SUCCESS)) {

		status = store_variable_data(context, info, var);
	}

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

	if (info) {

		/* Variable already exists */
		status = check_access_permitted(context, info);

		if (status == EFI_SUCCESS) {

			status = load_variable_data(context, info, var, max_data_len);
			var->Attributes = info->attributes;
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

	status = EFI_NOT_FOUND;
	*total_length = 0;

	const struct variable_info *info = variable_index_find_next(
		&context->variable_index,
		&cur->Guid,
		cur->NameSize,
		cur->Name);

	if (info && (info->name_size <= max_name_len)) {

		cur->Guid = info->guid;
		cur->NameSize = info->name_size;
		memcpy(cur->Name, info->name, info->name_size);

		*total_length = SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME_TOTAL_SIZE(cur);

		status = EFI_SUCCESS;
	}

	return status;
}

efi_status_t uefi_variable_store_query_variable_info(
	struct uefi_variable_store *context,
	SMM_VARIABLE_COMMUNICATE_QUERY_VARIABLE_INFO *cur)
{
	efi_status_t status = EFI_UNSUPPORTED;


	return status;
}

efi_status_t uefi_variable_store_exit_boot_service(
	struct uefi_variable_store *context)
{
	context->is_boot_service = false;
	return EFI_SUCCESS;
}

static void load_variable_index(
	struct uefi_variable_store *context)
{
	struct storage_backend *persistent_store = context->persistent_store;

	if (persistent_store) {

		size_t data_len = 0;

		psa_status_t psa_status = persistent_store->interface->get(
			persistent_store->context,
			context->owner_id,
			VARIABLE_INDEX_STORAGE_UID,
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

		struct storage_backend *persistent_store = context->persistent_store;

		if (persistent_store) {

			psa_status_t psa_status = persistent_store->interface->set(
				persistent_store->context,
				context->owner_id,
				VARIABLE_INDEX_STORAGE_UID,
				data_len,
				context->index_sync_buffer,
				PSA_STORAGE_FLAG_NONE);

			status = psa_to_efi_storage_status(psa_status);
		}
	}

	return status;
}

static efi_status_t check_access_permitted(
	const struct uefi_variable_store *context,
	const struct variable_info *info)
{
	efi_status_t status = EFI_SUCCESS;

	if (info->attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS) {

		if (!context->is_boot_service) status = EFI_ACCESS_DENIED;
	}
	else if (info->attributes & EFI_VARIABLE_RUNTIME_ACCESS) {

		if (context->is_boot_service) status = EFI_ACCESS_DENIED;
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

	bool is_nv = (info->attributes & EFI_VARIABLE_NON_VOLATILE);

	struct storage_backend *storage_backend = (is_nv) ?
		context->persistent_store :
		context->volatile_store;

	if (storage_backend) {

		psa_status = storage_backend->interface->set(
			storage_backend->context,
			context->owner_id,
			info->uid,
			data_len,
			data,
			PSA_STORAGE_FLAG_NONE);
	}

	if ((psa_status != PSA_SUCCESS) && is_nv) {

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
	bool is_nv = (info->attributes & EFI_VARIABLE_NON_VOLATILE);

	struct storage_backend *storage_backend = (is_nv) ?
		context->persistent_store :
		context->volatile_store;

	if (storage_backend) {

		psa_status = storage_backend->interface->remove(
			storage_backend->context,
			context->owner_id,
			info->uid);
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

	bool is_nv = (info->attributes & EFI_VARIABLE_NON_VOLATILE);

	struct storage_backend *storage_backend = (is_nv) ?
		context->persistent_store :
		context->volatile_store;

	if (storage_backend) {

		psa_status = storage_backend->interface->get(
			storage_backend->context,
			context->owner_id,
			info->uid,
			0,
			max_data_len,
			data,
			&data_len);

		var->DataSize = data_len;
	}

	return psa_to_efi_storage_status(psa_status);
}

static void purge_orphan_index_entries(
	struct uefi_variable_store *context)
{
	bool any_orphans = false;
	struct variable_index_iterator iter;
	variable_index_iterator_first(&iter, &context->variable_index);

	/* Iterate over variable index looking for any entries for NV
	 * variables where there is no corresponding object in the
	 * persistent store. This condition could arrise due to
	 * a power failure before an object is stored.
	 */
	while (!variable_index_iterator_is_done(&iter)) {

		const struct variable_info *info = variable_index_iterator_current(&iter);

		if (info->attributes & EFI_VARIABLE_NON_VOLATILE) {

			struct psa_storage_info_t storage_info;
			struct storage_backend *storage_backend = context->persistent_store;

			psa_status_t psa_status = storage_backend->interface->get_info(
				storage_backend->context,
				context->owner_id,
				info->uid,
				&storage_info);

			if (psa_status != PSA_SUCCESS) {

				/* Detected a mismatch between the index and storage */
				variable_index_remove(&context->variable_index, info);
				any_orphans = true;
			}
		}

		variable_index_iterator_next(&iter);
	}

	if (any_orphans) sync_variable_index(context);
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

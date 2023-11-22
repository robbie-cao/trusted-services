/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "plat_fmp_service.h"
#include <psa/client.h>
#include <psa/sid.h>
#include <psa/storage_common.h>
#include <trace.h>

#include <service/smm_variable/backend/variable_index.h>

#define VARIABLE_INDEX_STORAGE_UID			(0x787)

/**
 * Variable attributes
 */
#define	EFI_VARIABLE_NON_VOLATILE				(0x00000001)
#define	EFI_VARIABLE_BOOTSERVICE_ACCESS				(0x00000002)
#define	EFI_VARIABLE_RUNTIME_ACCESS				(0x00000004)
#define	EFI_VARIABLE_HARDWARE_ERROR_RECORD			(0x00000008)
#define	EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS			(0x00000010)
#define	EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS	(0x00000020)
#define	EFI_VARIABLE_APPEND_WRITE				(0x00000040)
#define	EFI_VARIABLE_MASK \
	(EFI_VARIABLE_NON_VOLATILE | \
	 EFI_VARIABLE_BOOTSERVICE_ACCESS | \
	 EFI_VARIABLE_RUNTIME_ACCESS | \
	 EFI_VARIABLE_HARDWARE_ERROR_RECORD | \
	 EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS | \
	 EFI_VARIABLE_APPEND_WRITE)

#define FMP_VARIABLES_COUNT	6

static struct variable_metadata fmp_variables_metadata[FMP_VARIABLES_COUNT] = {
    {
	{ 0x86c77a67, 0x0b97, 0x4633, \
		{ 0xa1, 0x87, 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7} },
	/* name size = (variable_name + \0) * sizeof(u16) */
	.name_size = 42, { 'F', 'm', 'p', 'D', 'e', 's', 'c', 'r', 'i', 'p', 't', 'o', 'r', 'V', 'e', 'r', 's', 'i', 'o', 'n' },
	.attributes = EFI_VARIABLE_NON_VOLATILE, .uid = 0
    },
    {
	{ 0x86c77a67, 0x0b97, 0x4633, \
		{ 0xa1, 0x87, 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7} },
	/* name size = (variable_name + \0) * sizeof(u16) */
	.name_size = 34, { 'F', 'm', 'p', 'I', 'm', 'a', 'g', 'e', 'I', 'n', 'f', 'o', 'S', 'i', 'z', 'e' },
	.attributes = EFI_VARIABLE_NON_VOLATILE, .uid = 0
    },
    {
	{ 0x86c77a67, 0x0b97, 0x4633, \
		{ 0xa1, 0x87, 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7} },
	/* name size = (variable_name + \0) * sizeof(u16) */
	.name_size = 38, { 'F', 'm', 'p', 'D', 'e', 's', 'c', 'r', 'i', 'p', 't', 'o', 'r', 'C', 'o', 'u', 'n', 't' },
	.attributes = EFI_VARIABLE_NON_VOLATILE, .uid = 0
    },
    {
	{ 0x86c77a67, 0x0b97, 0x4633, \
		{ 0xa1, 0x87, 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7} },
	/* name size = (variable_name + \0) * sizeof(u16) */
	.name_size = 26, { 'F', 'm', 'p', 'I', 'm', 'a', 'g', 'e', 'I', 'n', 'f', 'o' },
	.attributes = EFI_VARIABLE_NON_VOLATILE, .uid = 0
    },
    {
	{ 0x86c77a67, 0x0b97, 0x4633, \
		{ 0xa1, 0x87, 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7} },
	/* name size = (variable_name + \0) * sizeof(u16) */
	.name_size = 28, { 'F', 'm', 'p', 'I', 'm', 'a', 'g', 'e', 'N', 'a', 'm', 'e', '1' },
	.attributes = EFI_VARIABLE_NON_VOLATILE, .uid = 0
    },
    {
	{ 0x86c77a67, 0x0b97, 0x4633, \
		{ 0xa1, 0x87, 0x49, 0x10, 0x4d, 0x06, 0x85, 0xc7} },
	/* name size = (variable_name + \0) * sizeof(u16) */
	.name_size = 32, { 'F', 'm', 'p', 'V', 'e', 'r', 's', 'i', 'o', 'n',  'N', 'a', 'm', 'e', '1' },
	.attributes = EFI_VARIABLE_NON_VOLATILE, .uid = 0
    },
};

static psa_status_t protected_storage_set(struct rpc_caller *caller,
	psa_storage_uid_t uid, size_t data_length, const void *p_data)
{
	psa_status_t psa_status;
	psa_storage_create_flags_t create_flags = PSA_STORAGE_FLAG_NONE;

	struct psa_invec in_vec[] = {
		{ .base = psa_ptr_to_u32(&uid), .len = sizeof(uid) },
		{ .base = psa_ptr_const_to_u32(p_data), .len = data_length },
		{ .base = psa_ptr_to_u32(&create_flags), .len = sizeof(create_flags) },
	};

	psa_status = psa_call(caller, TFM_PROTECTED_STORAGE_SERVICE_HANDLE, TFM_PS_ITS_SET,
			      in_vec, IOVEC_LEN(in_vec), NULL, 0);
	if (psa_status < 0)
		EMSG("ipc_set: psa_call failed: %d", psa_status);

	return psa_status;
}

static psa_status_t protected_storage_get(struct rpc_caller *caller,
	psa_storage_uid_t uid, size_t data_size, void *p_data)
{
	psa_status_t psa_status;
	uint32_t offset = 0;

	struct psa_invec in_vec[] = {
		{ .base = psa_ptr_to_u32(&uid), .len = sizeof(uid) },
		{ .base = psa_ptr_to_u32(&offset), .len = sizeof(offset) },
	};

	struct psa_outvec out_vec[] = {
		{ .base = psa_ptr_to_u32(p_data), .len = data_size },
	};

	psa_status = psa_call(caller, TFM_PROTECTED_STORAGE_SERVICE_HANDLE,
			      TFM_PS_ITS_GET, in_vec, IOVEC_LEN(in_vec),
			      out_vec, IOVEC_LEN(out_vec));

	if (psa_status == PSA_SUCCESS && out_vec[0].len != data_size) {
	    EMSG("Return size does not match with expected size.");
	    return PSA_ERROR_BUFFER_TOO_SMALL;
	}

	return psa_status;
}	

static uint64_t name_hash(EFI_GUID *guid, size_t name_size,
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


static void initialize_metadata(void)
{
    for (int i = 0; i < FMP_VARIABLES_COUNT; i++) {

	fmp_variables_metadata[i].uid = name_hash(
					&fmp_variables_metadata[i].guid,
					fmp_variables_metadata[i].name_size,
					fmp_variables_metadata[i].name);
    }
}


void provision_fmp_variables_metadata(struct rpc_caller *caller)
{
    struct variable_metadata metadata;
    psa_status_t status;
    uint32_t dummy_values = 0xDEAD;

    EMSG("Provisioning FMP metadata.");

    initialize_metadata();

    status = protected_storage_get(caller, VARIABLE_INDEX_STORAGE_UID,
		sizeof(struct variable_metadata), &metadata);

    if (status == PSA_SUCCESS) {
	EMSG("UEFI variables store is already provisioned.");
	return;
    }

    /* Provision FMP variables with dummy values. */
    for (int i = 0; i < FMP_VARIABLES_COUNT; i++) {
	protected_storage_set(caller, fmp_variables_metadata[i].uid,
				sizeof(dummy_values), &dummy_values);
    }

    status = protected_storage_set(caller, VARIABLE_INDEX_STORAGE_UID,
		    sizeof(struct variable_metadata) * FMP_VARIABLES_COUNT,
		    fmp_variables_metadata);

    if (status != EFI_SUCCESS) {
	return;
    }

    EMSG("FMP metadata is provisioned");
}

typedef struct {
    void *base;
    int len;
} variable_data_t;

static variable_data_t fmp_variables_data[FMP_VARIABLES_COUNT];

#define IMAGE_INFO_BUFFER_SIZE	256
static char image_info_buffer[IMAGE_INFO_BUFFER_SIZE];
#define IOCTL_PLAT_FMP_IMAGE_INFO	2

static psa_status_t unpack_image_info(void *buffer, uint32_t size)
{
    typedef struct __attribute__ ((__packed__)) {
	uint32_t variable_count;
	uint32_t variable_size[FMP_VARIABLES_COUNT];
	uint8_t variable[];
    } packed_buffer_t;

    packed_buffer_t *packed_buffer = buffer;
    int runner = 0;

    if (packed_buffer->variable_count != FMP_VARIABLES_COUNT) {
	EMSG("Expected fmp varaibles = %u, but received = %u",
		FMP_VARIABLES_COUNT, packed_buffer->variable_count);
	return PSA_ERROR_PROGRAMMER_ERROR;
    }

    for (int i = 0; i < packed_buffer->variable_count; i++) {
	EMSG("FMP variable %d : size %u", i, packed_buffer->variable_size[i]);
	fmp_variables_data[i].base = &packed_buffer->variable[runner];
	fmp_variables_data[i].len= packed_buffer->variable_size[i];
	runner += packed_buffer->variable_size[i];
    }

    return PSA_SUCCESS;
}

static psa_status_t get_image_info(struct rpc_caller *caller)
{
    psa_status_t status;
    psa_handle_t handle;
    uint32_t ioctl_id = IOCTL_PLAT_FMP_IMAGE_INFO;

    struct psa_invec in_vec[] = {
	{ .base = &ioctl_id, .len = sizeof(ioctl_id) },
    };

    struct psa_outvec out_vec[] = {
	{ .base = image_info_buffer, .len = IMAGE_INFO_BUFFER_SIZE },
    };

    memset(image_info_buffer, 0, IMAGE_INFO_BUFFER_SIZE);

    psa_call(caller, TFM_PLATFORM_SERVICE_HANDLE, TFM_PLATFORM_API_ID_IOCTL,
	     in_vec, IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));

    status = unpack_image_info(image_info_buffer, IMAGE_INFO_BUFFER_SIZE);
    if (status != PSA_SUCCESS) {
	return status;
    }

    return PSA_SUCCESS;
}

static psa_status_t set_image_info(struct rpc_caller *caller)
{
    psa_status_t status;

    for (int i = 0; i < FMP_VARIABLES_COUNT; i++) {
	
	status = protected_storage_set(caller,
			fmp_variables_metadata[i].uid,
			fmp_variables_data[i].len, fmp_variables_data[i].base);

	if (status != PSA_SUCCESS) {

            EMSG("FMP variable %d set unsuccessful", i);
	    return status;
	}

        EMSG("FMP variable %d set success", i);
    }

    return PSA_SUCCESS;
}

void set_fmp_image_info(struct rpc_caller *caller)
{
    psa_status_t status;

    status = get_image_info(caller);
    if (status != PSA_SUCCESS) {
	return;
    }

    status = set_image_info(caller);
    if (status != PSA_SUCCESS) {
	return;
    }

    return;
}

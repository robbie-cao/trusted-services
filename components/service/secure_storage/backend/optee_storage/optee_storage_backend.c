// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2022, Arm Limited.
 */

#include "optee_storage_backend.h"
#include "tee/tee_svc_cryp.h"
#include "tee/tee_svc_storage.h"
#include <string.h>
#include <trace.h>

struct optee_storage_context {
	/* Storing storage ID with select RPMB or REE as the NWd backend */
	unsigned long storage_id;
	uint8_t *shared_buffer;
	size_t shared_buffer_size;
	size_t shared_buffer_used;
};

/**
 * @brief The unique identifier of a storage object is make of the identifier
 * passed as the function argument and the client's ID.
 *
 */
struct optee_storage_id {
	uint16_t client_id;
	uint64_t uid;
} __packed;

struct optee_storage_header {
	uint64_t capacity;
	uint64_t size;
	uint32_t flags;
	uint8_t reserved[12];
} __packed;

#define VALID_FLAGS                                                            \
	(PSA_STORAGE_FLAG_WRITE_ONCE | PSA_STORAGE_FLAG_NO_CONFIDENTIALITY |   \
	 PSA_STORAGE_FLAG_NO_REPLAY_PROTECTION)
#define IS_VALID_FLAG(flag) (((flag) & ~VALID_FLAGS) == 0)

#define TEE_DATA_FLAG_ACCESS_RW                                                \
	(TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE)

/**
 * This is the maximum size that should be allocated from the shared memory.
 */
#define SHARED_BUFFER_SHADOW_SIZE                                              \
	(sizeof(uint32_t) + sizeof(struct optee_storage_id) +                  \
	 sizeof(struct optee_storage_header))

static struct storage_backend optee_storage_backend;
static struct optee_storage_context optee_storage_context;
static uint8_t shared_buffer_shadow[SHARED_BUFFER_SHADOW_SIZE];

static psa_status_t get_psa_status(TEE_Result tee_result)
{
	switch (tee_result) {
	case TEE_SUCCESS:
		return PSA_SUCCESS;

	case TEE_ERROR_OVERFLOW:
		return PSA_ERROR_INVALID_ARGUMENT;

	case TEE_ERROR_ITEM_NOT_FOUND:
		return PSA_ERROR_DOES_NOT_EXIST;

	case TEE_ERROR_STORAGE_NOT_AVAILABLE:
		return PSA_ERROR_STORAGE_FAILURE;

	case TEE_ERROR_CORRUPT_OBJECT:
		return PSA_ERROR_DATA_CORRUPT;

	case TEE_ERROR_ACCESS_DENIED:
		return PSA_ERROR_NOT_PERMITTED;

	case TEE_ERROR_OUT_OF_MEMORY:
		return PSA_ERROR_INSUFFICIENT_STORAGE;

	case TEE_ERROR_ACCESS_CONFLICT:
		return PSA_ERROR_ALREADY_EXISTS;

	default:
		EMSG("Unknown storage error: 0x%x", tee_result);
		return PSA_ERROR_GENERIC_ERROR;
	}
}

/**
 * OP-TEE storage functions need some variables to be accessible from SEL0.
 * These functions allocated memory from the shared buffer so these checks can
 * pass.
 */
static void *shared_mem_alloc(struct optee_storage_context *ctx, size_t size)
{
	size_t used = ctx->shared_buffer_used;
	size_t end = used + size;

	if (ctx->shared_buffer_size < end || sizeof(shared_buffer_shadow) < end)
		return NULL;

	memcpy(shared_buffer_shadow + used, ctx->shared_buffer + used, size);
	ctx->shared_buffer_used += size;
	memset(ctx->shared_buffer + used, 0x00, size);

	return ctx->shared_buffer + used;
}

static void shared_mem_cleanup(struct optee_storage_context *ctx)
{
	memcpy(ctx->shared_buffer, shared_buffer_shadow,
	       ctx->shared_buffer_used);
	ctx->shared_buffer_used = 0;
}

static void *shared_mem_get_addr_by_offset(struct optee_storage_context *ctx,
					   size_t offset, size_t size)
{
	/* Align offset to n*size */
	offset = ROUNDUP(offset, size);

	if (ctx->shared_buffer_size < offset + size)
		return NULL;

	return ctx->shared_buffer + offset;
}

static TEE_Result open_storage_object(void *context, uint32_t client_id,
				      uint64_t uid, unsigned long flags,
				      uint32_t *obj)
{
	struct optee_storage_context *ctx =
		(struct optee_storage_context *)context;
	struct optee_storage_id *object_id = NULL;
	uint32_t *obj_internal = NULL;
	TEE_Result res = TEE_SUCCESS;

	object_id = (struct optee_storage_id *)shared_mem_alloc(ctx, sizeof(*object_id));
	if (!object_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	obj_internal = (uint32_t *)shared_mem_alloc(ctx, sizeof(*obj_internal));
	if (!obj_internal) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto return_error;
	}

	object_id->uid = uid;
	object_id->client_id = client_id;

	res = syscall_storage_obj_open(ctx->storage_id, object_id,
				       sizeof(*object_id), flags, obj_internal);

	if (res == TEE_SUCCESS)
		*obj = *obj_internal;

return_error:
	shared_mem_cleanup(ctx);

	return res;
}

static TEE_Result create_storage_object(void *context, uint32_t client_id,
					uint64_t uid, unsigned long flags,
					void *data, size_t data_length,
					uint32_t *obj)
{
	struct optee_storage_context *ctx =
		(struct optee_storage_context *)context;
	struct optee_storage_id *object_id = NULL;
	uint32_t *obj_internal = NULL;
	void *data_internal = NULL;
	TEE_Result res = TEE_SUCCESS;

	object_id =
		(struct optee_storage_id *)shared_mem_alloc(ctx, sizeof(struct optee_storage_id));
	if (!object_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	obj_internal = (uint32_t *)shared_mem_alloc(ctx, sizeof(*obj_internal));
	if (!obj_internal) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto return_error;
	};

	data_internal = shared_mem_alloc(ctx, data_length);
	if (!data_internal) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto return_error;
	}

	object_id->uid = uid;
	object_id->client_id = client_id;

	memcpy(data_internal, data, data_length);

	res = syscall_storage_obj_create(ctx->storage_id, object_id,
					 sizeof(*object_id), flags,
					 TEE_HANDLE_NULL, data_internal,
					 data_length, obj_internal);
	if (res == TEE_SUCCESS)
		*obj = *obj_internal;

return_error:
	shared_mem_cleanup(ctx);

	return res;
}

static TEE_Result get_header(void *context, uint32_t obj,
			     struct optee_storage_header *header)
{
	struct optee_storage_context *ctx =
		(struct optee_storage_context *)context;
	TEE_Result res = TEE_SUCCESS;
	uint64_t *count = NULL;
	struct optee_storage_header *header_internal = NULL;

	header_internal =
		(struct optee_storage_header *)shared_mem_alloc(ctx, sizeof(*header_internal));
	if (!header_internal)
		return TEE_ERROR_OUT_OF_MEMORY;

	count = (uint64_t *)shared_mem_alloc(ctx, sizeof(*count));
	if (!count) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto return_error;
	}

	res = syscall_storage_obj_seek(obj, 0, TEE_DATA_SEEK_SET);
	if (res != TEE_SUCCESS)
		goto return_error;

	res = syscall_storage_obj_read(obj, header_internal,
				       sizeof(*header_internal), count);
	if (res != TEE_SUCCESS)
		goto return_error;

	if (*count != sizeof(*header))
		res = TEE_ERROR_GENERIC;

	memcpy(header, header_internal, sizeof(*header));

return_error:
	shared_mem_cleanup(ctx);

	return res;
}

static TEE_Result set_header(void *context, uint32_t obj,
			     struct optee_storage_header *header)
{
	struct optee_storage_context *ctx =
		(struct optee_storage_context *)context;
	TEE_Result res = TEE_SUCCESS;
	struct optee_storage_header *header_internal = NULL;

	header_internal =
		(struct optee_storage_header *)shared_mem_alloc(ctx, sizeof(*header_internal));
	if (!header_internal)
		return TEE_ERROR_OUT_OF_MEMORY;

	memcpy(header_internal, header, sizeof(*header_internal));

	res = syscall_storage_obj_seek(obj, 0, TEE_DATA_SEEK_SET);
	if (res != TEE_SUCCESS)
		goto return_error;

	res = syscall_storage_obj_write(obj, header_internal,
					sizeof(*header_internal));

return_error:
	shared_mem_cleanup(ctx);

	return res;
}

static psa_status_t optee_storage_set(void *context, uint32_t client_id,
				      uint64_t uid, size_t data_length,
				      const void *p_data, uint32_t create_flags)
{
	TEE_Result res = TEE_SUCCESS;
	TEE_Result res_close = TEE_SUCCESS;
	uint32_t obj = 0;
	struct optee_storage_header storage_header = { 0 };

	if (uid == 0)
		return PSA_ERROR_INVALID_ARGUMENT;

	if (!IS_VALID_FLAG(create_flags))
		return PSA_ERROR_INVALID_ARGUMENT;

	res = open_storage_object(context, client_id, uid,
				  TEE_DATA_FLAG_ACCESS_RW, &obj);
	if (res == TEE_SUCCESS) {
		/* The object exists, read the header */
		res = get_header(context, obj, &storage_header);
		if (res != TEE_SUCCESS)
			goto close_and_return_result;

		if (storage_header.flags & PSA_STORAGE_FLAG_WRITE_ONCE) {
			res = TEE_ERROR_ACCESS_DENIED;
			goto close_and_return_result;
		}

		storage_header.capacity = data_length;
		storage_header.size = data_length;
		storage_header.flags = create_flags;

		res = set_header(context, obj, &storage_header);
	} else if (res == TEE_ERROR_ITEM_NOT_FOUND) {
		/* The object does not exist, create a new one. */
		storage_header.capacity = data_length;
		storage_header.size = data_length;
		storage_header.flags = create_flags;

		res = create_storage_object(context, client_id, uid,
					    TEE_DATA_FLAG_ACCESS_RW,
					    &storage_header,
					    sizeof(storage_header), &obj);
	}

	if (res != TEE_SUCCESS) {
		/* Fatal error happened during open or create */
		goto return_result;
	}

	res = syscall_storage_obj_seek(obj, sizeof(storage_header),
				       TEE_DATA_SEEK_SET);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	res = syscall_storage_obj_write(obj, (void *)p_data, data_length);

close_and_return_result:
	res_close = syscall_cryp_obj_close(obj);

return_result:
	return get_psa_status(res != TEE_SUCCESS ? res : res_close);
}

static psa_status_t optee_storage_get(void *context, uint32_t client_id,
				      uint64_t uid, size_t data_offset,
				      size_t data_size, void *p_data,
				      size_t *p_data_length)
{
	struct optee_storage_context *ctx =
		(struct optee_storage_context *)context;
	TEE_Result res = TEE_SUCCESS;
	TEE_Result res_close = TEE_SUCCESS;
	uint32_t obj = 0;
	struct optee_storage_header storage_header = { 0 };
	int32_t data_start_index = 0;
	size_t *p_data_length_internal = NULL;

	if (uid == 0)
		return PSA_ERROR_INVALID_ARGUMENT;

	res = open_storage_object(context, client_id, uid,
				  TEE_DATA_FLAG_ACCESS_READ, &obj);
	if (res != TEE_SUCCESS)
		goto return_result;

	res = get_header(context, obj, &storage_header);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	/* Validating data_offset */
	if (data_offset > storage_header.size ||
	    ADD_OVERFLOW(data_offset, sizeof(storage_header),
			 &data_start_index)) {
		res = TEE_ERROR_OVERFLOW;
		goto close_and_return_result;
	}

	/* Limiting data_size */
	data_size = MIN(data_size, storage_header.size - data_offset);

	res = syscall_storage_obj_seek(obj, data_start_index,
				       TEE_DATA_SEEK_SET);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	/**
	 * This allocation has to be different because otherwise the data would
	 * be overwritten by the returned data length. In this case the length
	 * variable is allocated after the data area of the shared memory.
	 */
	p_data_length_internal = (size_t *)shared_mem_get_addr_by_offset(ctx, data_size,
									 sizeof(size_t));
	if (!p_data_length_internal) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto close_and_return_result;
	}

	res = syscall_storage_obj_read(obj, p_data, data_size,
				       p_data_length_internal);
	if (res == TEE_SUCCESS) {
		*p_data_length = *p_data_length_internal;
		*p_data_length_internal = 0;
	}

close_and_return_result:
	res_close = syscall_cryp_obj_close(obj);

return_result:
	return get_psa_status(res != TEE_SUCCESS ? res : res_close);
}

static psa_status_t optee_storage_get_info(void *context, uint32_t client_id,
					   uint64_t uid,
					   struct psa_storage_info_t *p_info)
{
	TEE_Result res = TEE_SUCCESS;
	TEE_Result res_close = TEE_SUCCESS;
	uint32_t obj = 0;
	struct optee_storage_header storage_header = { 0 };

	if (uid == 0)
		return PSA_ERROR_INVALID_ARGUMENT;

	res = open_storage_object(context, client_id, uid,
				  TEE_DATA_FLAG_ACCESS_READ, &obj);
	if (res != TEE_SUCCESS)
		goto return_result;

	res = get_header(context, obj, &storage_header);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	p_info->capacity = storage_header.capacity;
	p_info->size = storage_header.size;
	p_info->flags = storage_header.flags;

close_and_return_result:
	res_close = syscall_cryp_obj_close(obj);

return_result:
	return get_psa_status(res != TEE_SUCCESS ? res : res_close);
}

static psa_status_t optee_storage_remove(void *context, uint32_t client_id,
					 uint64_t uid)
{
	TEE_Result res = TEE_SUCCESS;
	TEE_Result res_close = TEE_SUCCESS;
	uint32_t obj = 0;
	struct optee_storage_header storage_header = { 0 };

	if (uid == 0)
		return PSA_ERROR_INVALID_ARGUMENT;

	res = open_storage_object(context, client_id, uid,
				  TEE_DATA_FLAG_ACCESS_READ |
					  TEE_DATA_FLAG_ACCESS_WRITE_META,
				  &obj);
	if (res != TEE_SUCCESS)
		goto return_result;

	res = get_header(context, obj, &storage_header);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	if (storage_header.flags & PSA_STORAGE_FLAG_WRITE_ONCE) {
		res = TEE_ERROR_ACCESS_DENIED;
		goto close_and_return_result;
	}

	res = syscall_storage_obj_del(obj);
	if (res == TEE_SUCCESS)
		/* Skip close on successful delete */
		goto return_result;

close_and_return_result:
	res_close = syscall_cryp_obj_close(obj);

return_result:
	return get_psa_status(res != TEE_SUCCESS ? res : res_close);
}

static psa_status_t optee_storage_create(void *context, uint32_t client_id,
					 uint64_t uid, size_t capacity,
					 uint32_t create_flags)
{
	TEE_Result res = TEE_SUCCESS;
	TEE_Result res_close = TEE_SUCCESS;
	uint32_t obj = 0;
	struct optee_storage_header storage_header = { 0 };

	if (uid == 0)
		return PSA_ERROR_INVALID_ARGUMENT;

	if (!IS_VALID_FLAG(create_flags))
		return PSA_ERROR_INVALID_ARGUMENT;

	if (create_flags & PSA_STORAGE_FLAG_WRITE_ONCE)
		return PSA_ERROR_NOT_SUPPORTED;

	res = create_storage_object(context, client_id, uid,
				    TEE_DATA_FLAG_ACCESS_READ |
					    TEE_DATA_FLAG_ACCESS_WRITE,
				    NULL, 0, &obj);
	if (res != TEE_SUCCESS)
		goto return_result;

	storage_header.capacity = capacity;
	storage_header.size = 0;
	storage_header.flags = create_flags;

	res = set_header(context, obj, &storage_header);

	res_close = syscall_cryp_obj_close(obj);

return_result:
	return get_psa_status(res != TEE_SUCCESS ? res : res_close);
}

static psa_status_t optee_storage_set_extended(void *context,
					       uint32_t client_id, uint64_t uid,
					       size_t data_offset,
					       size_t data_length,
					       const void *p_data)
{
	TEE_Result res = TEE_SUCCESS;
	TEE_Result res_close = TEE_SUCCESS;
	uint32_t obj = 0;
	struct optee_storage_header storage_header = { 0 };
	int32_t data_start_index = 0;
	size_t data_end_index = 0;

	if (uid == 0)
		return PSA_ERROR_INVALID_ARGUMENT;

	res = open_storage_object(context, client_id, uid,
				  TEE_DATA_FLAG_ACCESS_RW, &obj);
	if (res != TEE_SUCCESS)
		goto return_result;

	res = get_header(context, obj, &storage_header);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	if (data_offset > storage_header.size ||
	    ADD_OVERFLOW(data_offset, data_length, &data_end_index) ||
	    data_end_index > storage_header.capacity ||
	    ADD_OVERFLOW(data_offset, sizeof(storage_header),
			 &data_start_index)) {
		res = TEE_ERROR_OVERFLOW;
		goto close_and_return_result;
	}

	res = syscall_storage_obj_seek(obj, data_start_index,
				       TEE_DATA_SEEK_SET);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	res = syscall_storage_obj_write(obj, (void *)p_data, data_length);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

	storage_header.size = MAX(storage_header.size, data_end_index);

	res = set_header(context, obj, &storage_header);
	if (res != TEE_SUCCESS)
		goto close_and_return_result;

close_and_return_result:
	res_close = syscall_cryp_obj_close(obj);

return_result:
	return get_psa_status(res != TEE_SUCCESS ? res : res_close);
}

static uint32_t optee_storage_get_support(void *context __unused,
					  uint32_t client_id __unused)
{
	return PSA_STORAGE_SUPPORT_SET_EXTENDED;
}

static struct storage_backend_interface optee_storage_interface = {
	.set = optee_storage_set,
	.get = optee_storage_get,
	.get_info = optee_storage_get_info,
	.remove = optee_storage_remove,
	.create = optee_storage_create,
	.set_extended = optee_storage_set_extended,
	.get_support = optee_storage_get_support
};

struct storage_backend *optee_storage_backend_init(unsigned long storage_id)
{
	optee_storage_context.storage_id = storage_id;
	optee_storage_backend.context = &optee_storage_context;
	optee_storage_backend.interface = &optee_storage_interface;

	return &optee_storage_backend;
}

void optee_storage_backend_assign_shared_buffer(struct storage_backend *backend,
						void *buffer,
						size_t buffer_size)
{
	struct optee_storage_context *context =
		(struct optee_storage_context *)backend->context;
	context->shared_buffer = (uint8_t *)buffer;
	context->shared_buffer_size = buffer_size;
	context->shared_buffer_used = 0;
}

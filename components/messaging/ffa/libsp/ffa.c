// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 */

#include <assert.h>            // for assert
#include <stddef.h>            // for size_t
#include <stdint.h>            // for uint32_t, uint16_t, uintptr_t, U
#include "ffa_api.h"           // for FFA_OK, ffa_interrupt_handler, ffa_fea...
#include "ffa_api_defines.h"   // for FFA_PARAM_MBZ, FFA_OK, FFA_ERROR, FFA_...
#include "ffa_api_types.h"     // for ffa_result, ffa_direct_msg, ffa_uuid
#include "ffa_internal_api.h"  // for ffa_params, ffa_svc
#include "util.h"              // for GENMASK_32, SHIFT_U32, BIT

/*
 * Unpacks the error code from the FFA_ERROR message. It is a signed 32 bit
 * value in an unsigned 64 bit field so proper casting must be used to avoid
 * compiler dependent behavior.
 */
static inline ffa_result ffa_get_errorcode(struct ffa_params *result)
{
	uint32_t raw_value = result->a2;

	return *(ffa_result *)(&raw_value);
}

/*
 * Unpacks the content of the SVC result into an ffa_direct_msg structure.
 */
static inline void ffa_unpack_direct_msg(struct ffa_params *svc_result,
					 struct ffa_direct_msg *msg)
{
	msg->function_id = svc_result->a0;
	msg->source_id = (svc_result->a1 >> 16);
	msg->destination_id = svc_result->a1;
	msg->args[0] = svc_result->a3;
	msg->args[1] = svc_result->a4;
	msg->args[2] = svc_result->a5;
	msg->args[3] = svc_result->a6;
	msg->args[4] = svc_result->a7;
}

/*
 * The end of the interrupt handler is indicated by an FFA_MSG_WAIT call.
 */
static inline void ffa_return_from_interrupt(struct ffa_params *result)
{
	ffa_svc(FFA_MSG_WAIT, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		result);
}

static inline void ffa_uuid_to_abi_format(const struct ffa_uuid *uuid,
					  uint32_t *result)
{
	size_t i = 0;

	for (i = 0; i < 4; i++) {
		result[i] = uuid->uuid[4 * i];
		result[i] |= SHIFT_U32(uuid->uuid[4 * i + 1], 8);
		result[i] |= SHIFT_U32(uuid->uuid[4 * i + 2], 16);
		result[i] |= SHIFT_U32(uuid->uuid[4 * i + 3], 24);
	}
}

ffa_result ffa_version(uint32_t *version)
{
	struct ffa_params result = {0};
	uint32_t self_version = 0;

	self_version = (FFA_VERSION_MAJOR << FFA_VERSION_MAJOR_SHIFT) |
		       (FFA_VERSION_MINOR << FFA_VERSION_MINOR);

	ffa_svc(FFA_VERSION, self_version, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		&result);

	if (result.a0 & BIT(31)) {
		uint32_t raw_error = result.a0;

		*version = 0;

		return *(ffa_result *)(&raw_error);
	}

	*version = result.a0;
	return FFA_OK;
}

ffa_result ffa_features(uint32_t ffa_function_id,
			struct ffa_interface_properties *interface_properties)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_FEATURES, ffa_function_id, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		&result);

	if (result.a0 == FFA_ERROR) {
		interface_properties->interface_properties[0] = 0;
		interface_properties->interface_properties[1] = 0;
		return ffa_get_errorcode(&result);
	}

	assert(result.a0 == FFA_SUCCESS_32);
	interface_properties->interface_properties[0] = result.a2;
	interface_properties->interface_properties[1] = result.a3;
	return FFA_OK;
}

ffa_result ffa_rx_release(void)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_RX_RELEASE, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		&result);

	if (result.a0 == FFA_ERROR)
		return ffa_get_errorcode(&result);

	assert(result.a0 == FFA_SUCCESS_32);
	return FFA_OK;
}

ffa_result ffa_rxtx_map(const void *tx_buffer, const void *rx_buffer,
			uint32_t page_count)
{
	struct ffa_params result = {0};

	assert(page_count <= FFA_RXTX_MAP_PAGE_COUNT_MAX);

	page_count = SHIFT_U32(page_count & FFA_RXTX_MAP_PAGE_COUNT_MASK,
			       FFA_RXTX_MAP_PAGE_COUNT_SHIFT);

	ffa_svc(FFA_RXTX_MAP_32, (uintptr_t)tx_buffer, (uintptr_t)rx_buffer,
		page_count, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, &result);

	if (result.a0 == FFA_ERROR)
		return ffa_get_errorcode(&result);

	assert(result.a0 == FFA_SUCCESS_32);
	return FFA_OK;
}

ffa_result ffa_rxtx_unmap(uint16_t id)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_RXTX_UNMAP, SHIFT_U32(id, FFA_RXTX_UNMAP_ID_SHIFT),
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, &result);

	if (result.a0 == FFA_ERROR)
		return ffa_get_errorcode(&result);

	assert(result.a0 == FFA_SUCCESS_32);
	return FFA_OK;
}

ffa_result ffa_partition_info_get(const struct ffa_uuid *uuid, uint32_t *count)
{
	struct ffa_params result = {0};
	uint32_t abi_uuid[4] = {0};

	ffa_uuid_to_abi_format(uuid, abi_uuid);

	ffa_svc(FFA_PARTITION_INFO_GET, abi_uuid[0], abi_uuid[1], abi_uuid[2],
		abi_uuid[3], FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		&result);

	if (result.a0 == FFA_ERROR) {
		*count = UINT32_C(0);
		return ffa_get_errorcode(&result);
	}

	assert(result.a0 == FFA_SUCCESS_32);
	*count = result.a2;
	return FFA_OK;
}

ffa_result ffa_id_get(uint16_t *id)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_ID_GET, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		&result);

	if (result.a0 == FFA_ERROR) {
		*id = FFA_ID_GET_ID_MASK;
		return ffa_get_errorcode(&result);
	}

	assert(result.a0 == FFA_SUCCESS_32);
	*id = (result.a2 >> FFA_ID_GET_ID_SHIFT) & FFA_ID_GET_ID_MASK;
	return FFA_OK;
}

ffa_result ffa_msg_wait(struct ffa_direct_msg *msg)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_MSG_WAIT, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		&result);

	while (result.a0 == FFA_INTERRUPT) {
		ffa_interrupt_handler(result.a2);
		ffa_return_from_interrupt(&result);
	}

	if (result.a0 == FFA_ERROR) {
		return ffa_get_errorcode(&result);
	} else if (result.a0 == FFA_MSG_SEND_DIRECT_REQ_32) {
		ffa_unpack_direct_msg(&result, msg);
	} else {
		assert(result.a0 == FFA_SUCCESS_32);
		*msg = (struct ffa_direct_msg){.function_id = result.a0};
	}

	return FFA_OK;
}

ffa_result ffa_msg_send_direct_req(uint16_t source, uint16_t dest, uint32_t a0,
				   uint32_t a1, uint32_t a2, uint32_t a3,
				   uint32_t a4, struct ffa_direct_msg *msg)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_MSG_SEND_DIRECT_REQ_32,
		SHIFT_U32(source, FFA_MSG_SEND_DIRECT_REQ_SOURCE_ID_SHIFT) |
		dest, FFA_PARAM_MBZ, a0, a1, a2, a3, a4, &result);

	while (result.a0 == FFA_INTERRUPT) {
		ffa_interrupt_handler(result.a2);
		ffa_return_from_interrupt(&result);
	}

	if (result.a0 == FFA_ERROR) {
		return ffa_get_errorcode(&result);
	} else if (result.a0 == FFA_MSG_SEND_DIRECT_RESP_32) {
		ffa_unpack_direct_msg(&result, msg);
	} else {
		assert(result.a0 == FFA_SUCCESS_32);
		*msg = (struct ffa_direct_msg){.function_id = result.a0};
	}

	return FFA_OK;
}

ffa_result ffa_msg_send_direct_resp(uint16_t source, uint16_t dest, uint32_t a0,
				    uint32_t a1, uint32_t a2, uint32_t a3,
				    uint32_t a4, struct ffa_direct_msg *msg)
{
	struct ffa_params result = {0};

	ffa_svc(FFA_MSG_SEND_DIRECT_RESP_32,
		SHIFT_U32(source, FFA_MSG_SEND_DIRECT_RESP_SOURCE_ID_SHIFT) |
		dest, FFA_PARAM_MBZ, a0, a1, a2, a3, a4, &result);

	while (result.a0 == FFA_INTERRUPT) {
		ffa_interrupt_handler(result.a2);
		ffa_return_from_interrupt(&result);
	}

	if (result.a0 == FFA_ERROR) {
		return ffa_get_errorcode(&result);
	} else if (result.a0 == FFA_MSG_SEND_DIRECT_REQ_32) {
		ffa_unpack_direct_msg(&result, msg);
	} else {
		assert(result.a0 == FFA_SUCCESS_32);
		*msg = (struct ffa_direct_msg){.function_id = result.a0};
	}

	return FFA_OK;
}

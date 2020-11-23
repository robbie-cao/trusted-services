/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mock_store_provider.h"
#include <protocols/service/secure_storage/packed-c/secure_storage_proto.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/service/psa/packed-c/status.h>
#include <stdlib.h>
#include <string.h>

static struct mock_store_slot *find_slot(struct mock_store_provider *context, uint32_t id);
static struct mock_store_slot *find_empty_slot(struct mock_store_provider *context);
static void free_slot(struct mock_store_slot *slot);
static rpc_status_t set_handler(void *context, struct call_req* req);
static rpc_status_t get_handler(void *context, struct call_req* req);
static rpc_status_t get_info_handler(void *context, struct call_req* req);
static rpc_status_t remove_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
	{TS_SECURE_STORAGE_OPCODE_SET,      set_handler},
	{TS_SECURE_STORAGE_OPCODE_GET,	    get_handler},
	{TS_SECURE_STORAGE_OPCODE_GET_INFO,	get_info_handler},
	{TS_SECURE_STORAGE_OPCODE_REMOVE,	remove_handler}
};

struct call_ep *mock_store_provider_init(struct mock_store_provider *context)
{
    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i) {

        context->slots[i].len = 0;
        context->slots[i].flags = 0;
        context->slots[i].id = (uint32_t)(-1);
        context->slots[i].item = NULL;
    }

    service_provider_init(&context->base_provider, context,
                    handler_table, sizeof(handler_table)/sizeof(struct service_handler));

    return service_provider_get_call_ep(&context->base_provider);
}

void mock_store_provider_deinit(struct mock_store_provider *context)
{
    mock_store_reset(context);
}

void mock_store_reset(struct mock_store_provider *context)
{
    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i)
        free_slot(&context->slots[i]);
}

bool mock_store_exists(const struct mock_store_provider *context, uint32_t id)
{
    bool exists = false;

    for (int i = 0; !exists && i < MOCK_STORE_NUM_SLOTS; ++i) {
        exists = context->slots[i].item && (context->slots[i].id == id);
    }

    return exists;
}

size_t mock_store_num_items(const struct mock_store_provider *context)
{
    size_t count = 0;

    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i) {
        if (context->slots[i].item) ++count;
    }

    return count;
}

static struct mock_store_slot *find_slot(struct mock_store_provider *context, uint32_t id)
{
    struct mock_store_slot *slot = NULL;

    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i) {
        if (context->slots[i].item && (context->slots[i].id == id)) {
            slot = &context->slots[i];
            break;
        }
    }

    return slot;
}

static struct mock_store_slot *find_empty_slot(struct mock_store_provider *context)
{
    struct mock_store_slot *slot = NULL;

    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i) {
        if (!context->slots[i].item) {
            slot = &context->slots[i];
            break;
        }
    }

    return slot;
}

static void free_slot(struct mock_store_slot *slot)
{
    if (slot->item) {
        free(slot->item);
        slot->len = 0;
        slot->flags = 0;
        slot->id = (uint32_t)(-1);
        slot->item = NULL;
    }
}

static rpc_status_t set_handler(void *context, struct call_req *req)
{
    psa_status_t psa_status = PSA_ERROR_INSUFFICIENT_MEMORY;
    struct mock_store_provider *this_context = (struct mock_store_provider*)context;
    struct mock_store_slot *slot;
  	struct secure_storage_request_set *request_desc;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_set))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_set *)(req->req_buf.data);

	/* Checking for overflow */
	if (sizeof(struct secure_storage_request_set) + request_desc->data_length < request_desc->data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	/* Checking if descriptor and data fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_set) + request_desc->data_length)
		return TS_RPC_ERROR_INVALID_REQ_BODY;

    /* Replace existing or add new item */
    slot = find_slot(this_context, request_desc->uid);
    if (slot) free_slot(slot);
    else slot = find_empty_slot(this_context);

    if (slot) {
        slot->id = request_desc->uid;
        slot->flags = request_desc->create_flags;
        slot->len = request_desc->data_length;
        slot->item = malloc(slot->len);
        if (slot->item) {
            memcpy(slot->item, request_desc->p_data, slot->len);
            psa_status = PSA_SUCCESS;
        }
    }

	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_handler(void *context, struct call_req *req)
{
	struct mock_store_provider *this_context = (struct mock_store_provider*)context;
    struct secure_storage_request_get *request_desc;
	psa_status_t psa_status = PSA_ERROR_DOES_NOT_EXIST;
    struct mock_store_slot *slot;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_get))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_get *)(req->req_buf.data);

	/* Check if the requested data would fit into the response buffer. */
	if (req->resp_buf.size < request_desc->data_size)
		return TS_RPC_ERROR_INVALID_RESP_BODY;

    /* Find the item */
    slot = find_slot(this_context, request_desc->uid);

    if (slot && (slot->len <= req->resp_buf.size)) {
        memcpy(req->resp_buf.data, slot->item, slot->len);
        req->resp_buf.data_len = slot->len;
        psa_status = PSA_SUCCESS;
    }

	call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t get_info_handler(void *context, struct call_req *req)
{
 	struct mock_store_provider *this_context = (struct mock_store_provider*)context;
    struct secure_storage_request_get_info *request_desc;
	struct secure_storage_response_get_info *response_desc;
	psa_status_t psa_status;
    struct mock_store_slot *slot;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_get_info))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_get_info *)(req->req_buf.data);

	/* Checking if the response structure would fit the response buffer */
	if (req->resp_buf.size < sizeof(struct secure_storage_response_get_info))
		return TS_RPC_ERROR_INVALID_RESP_BODY;

    response_desc = (struct secure_storage_response_get_info *)(req->resp_buf.data);
    req->resp_buf.data_len = sizeof(struct secure_storage_response_get_info);

    /* Find itemto get info about */
    slot = find_slot(this_context, request_desc->uid);

    if (slot) {
        response_desc->capacity = slot->len;
        response_desc->size = slot->len;
        response_desc->flags = slot->flags;
        psa_status = PSA_SUCCESS;
    }
    else {
        response_desc->capacity = 0;
        response_desc->size = 0;
        response_desc->flags = 0;
        psa_status = PSA_ERROR_DOES_NOT_EXIST;
    }

    call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}

static rpc_status_t remove_handler(void *context, struct call_req *req)
{
	struct mock_store_provider *this_context = (struct mock_store_provider*)context;
    struct secure_storage_request_remove *request_desc;
	psa_status_t psa_status = PSA_ERROR_DOES_NOT_EXIST;
    struct mock_store_slot *slot;

	/* Checking if the descriptor fits into the request buffer */
	if (req->req_buf.data_len < sizeof(struct secure_storage_request_remove))
		return TS_RPC_ERROR_INVALID_REQ_BODY;

	request_desc = (struct secure_storage_request_remove *)(req->req_buf.data);

    /* Find and remove the item */
    slot = find_slot(this_context, request_desc->uid);

    if (slot) {
        free_slot(slot);
        psa_status = PSA_SUCCESS;
    }

    call_req_set_opstatus(req, psa_status);

	return TS_RPC_CALL_ACCEPTED;
}
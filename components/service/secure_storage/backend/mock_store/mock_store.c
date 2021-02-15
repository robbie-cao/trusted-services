/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mock_store.h"
#include <protocols/service/psa/packed-c/status.h>
#include <stdlib.h>
#include <string.h>

static struct mock_store_slot *find_slot(struct mock_store *context, uint32_t id);
static struct mock_store_slot *find_empty_slot(struct mock_store *context);
static void free_slot(struct mock_store_slot *slot);


static psa_status_t mock_store_set(void *context,
                            uint32_t client_id,
                            uint64_t uid,
                            size_t data_length,
                            const void *p_data,
                            uint32_t create_flags)
{
    psa_status_t psa_status = PSA_ERROR_INSUFFICIENT_MEMORY;
    struct mock_store *this_context = (struct mock_store*)context;

    /* Replace existing or add new item */
    struct mock_store_slot *slot = find_slot(this_context, uid);
    if (slot) free_slot(slot);
    else slot = find_empty_slot(this_context);

    if (slot) {
        slot->id = uid;
        slot->flags = create_flags;
        slot->len = data_length;
        slot->item = malloc(slot->len);
        if (slot->item) {
            memcpy(slot->item, p_data, slot->len);
            psa_status = PSA_SUCCESS;
        }
    }

    return psa_status;
}

static psa_status_t mock_store_get(void *context,
                            uint32_t client_id,
                            uint64_t uid,
                            size_t data_offset,
                            size_t data_size,
                            void *p_data,
                            size_t *p_data_length)
{
    psa_status_t psa_status = PSA_ERROR_DOES_NOT_EXIST;
    struct mock_store *this_context = (struct mock_store*)context;

    /* Find the item */
    struct mock_store_slot *slot = find_slot(this_context, uid);

    if (slot && (slot->len <= data_size)) {
        memcpy(p_data, slot->item, slot->len);
        *p_data_length = slot->len;
        psa_status = PSA_SUCCESS;
    }

    return psa_status;
}

static psa_status_t mock_store_get_info(void *context,
                            uint32_t client_id,
                            uint64_t uid,
                            struct psa_storage_info_t *p_info)
{
    psa_status_t psa_status = PSA_ERROR_DOES_NOT_EXIST;
    struct mock_store *this_context = (struct mock_store*)context;

    /* Find item to get info about */
    struct mock_store_slot *slot = find_slot(this_context, uid);

    if (slot) {
        p_info->capacity = slot->len;
        p_info->size = slot->len;
        p_info->flags = slot->flags;
        psa_status = PSA_SUCCESS;
    }
    else {
        p_info->capacity = 0;
        p_info->size = 0;
        p_info->flags = 0;
    }

    return psa_status;
}

static psa_status_t mock_store_remove(void *context,
                                uint32_t client_id,
                                uint64_t uid)
{
    psa_status_t psa_status = PSA_ERROR_DOES_NOT_EXIST;
    struct mock_store *this_context = (struct mock_store*)context;

    /* Find and remove the item */
    struct mock_store_slot *slot = find_slot(this_context, uid);

    if (slot) {
        free_slot(slot);
        psa_status = PSA_SUCCESS;
    }

    return psa_status;
}

struct storage_backend *mock_store_init(struct mock_store *context)
{
    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i) {

        context->slots[i].len = 0;
        context->slots[i].flags = 0;
        context->slots[i].id = (uint32_t)(-1);
        context->slots[i].item = NULL;
    }

    static const struct storage_backend_interface interface =
    {
        mock_store_set,
        mock_store_get,
        mock_store_get_info,
        mock_store_remove
    };

    context->backend.context = context;
    context->backend.interface = &interface;

    return &context->backend;
}

void mock_store_deinit(struct mock_store *context)
{
    mock_store_reset(context);
}

void mock_store_reset(struct mock_store *context)
{
    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i)
        free_slot(&context->slots[i]);
}

bool mock_store_exists(const struct mock_store *context, uint32_t id)
{
    bool exists = false;

    for (int i = 0; !exists && i < MOCK_STORE_NUM_SLOTS; ++i) {
        exists = context->slots[i].item && (context->slots[i].id == id);
    }

    return exists;
}

size_t mock_store_num_items(const struct mock_store *context)
{
    size_t count = 0;

    for (int i = 0; i < MOCK_STORE_NUM_SLOTS; ++i) {
        if (context->slots[i].item) ++count;
    }

    return count;
}

static struct mock_store_slot *find_slot(struct mock_store *context, uint32_t id)
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

static struct mock_store_slot *find_empty_slot(struct mock_store *context)
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
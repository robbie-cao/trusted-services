/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOCK_STORE_PROVIDER_H
#define MOCK_STORE_PROVIDER_H

#include <stdbool.h>
#include <stdint.h>
#include <service/common/provider/service_provider.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOCK_STORE_NUM_SLOTS        (100)

struct mock_store_slot
{
    uint64_t id;
    uint32_t flags;
    size_t len;
    uint8_t *item;
};

struct mock_store_provider
{
    struct service_provider base_provider;
    struct mock_store_slot slots[MOCK_STORE_NUM_SLOTS];
};

struct call_ep *mock_store_provider_init(struct mock_store_provider *context);
void mock_store_provider_deinit(struct mock_store_provider *context);

/* Test support methods */
void mock_store_reset(struct mock_store_provider *context);
bool mock_store_exists(const struct mock_store_provider *context, uint32_t id);
size_t mock_store_num_items(const struct mock_store_provider *context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MOCK_STORE_PROVIDER_H */

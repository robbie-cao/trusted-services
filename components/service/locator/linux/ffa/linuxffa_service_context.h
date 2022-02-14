/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LINUXFFA_SERVICE_CONTEXT_H
#define LINUXFFA_SERVICE_CONTEXT_H

#include <service_locator.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A service_context that represents a service instance located in
 * a partition, accessed via FFA.  This service_context is suitable
 * for use by client applications running in Linux userspace.
 */
struct linuxffa_service_context
{
    struct service_context service_context;
    char *ffa_dev_path;
    uint16_t partition_id;
    uint16_t iface_id;
};

/*
 * Factory method to create a service context associated with theh specified
 * partition id and RPC interface instance.
 */
struct linuxffa_service_context *linuxffa_service_context_create(const char *dev_path,
    uint16_t partition_id, uint16_t iface_id);

#ifdef __cplusplus
}
#endif

#endif /* LINUXFFA_SERVICE_CONTEXT_H */

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LOG_FACTORY_H
#define LOG_FACTORY_H

#include "components/rpc/common/interface/rpc_caller.h"
#include "components/service/log/common/log_status.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct log_backend;

struct log_backend *log_factory_create(void);
void log_factory_destroy(struct log_backend *backend);

#ifdef __cplusplus
}
#endif

#endif /* LOG_FACTORY_H */

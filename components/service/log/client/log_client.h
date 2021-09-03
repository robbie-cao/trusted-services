/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LOG_CLIENT_H
#define LOG_CLIENT_H

#include "components/rpc/common/interface/rpc_caller.h"
#include "components/service/log/common/log_status.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_client_init(struct rpc_caller *caller);
log_status_t log_client_puts(const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* LOG_CLIENT_H */

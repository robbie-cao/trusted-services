/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LOG_PROVIDER_H
#define LOG_PROVIDER_H

#include "components/service/common/provider/service_provider.h"

#ifdef __cplusplus
extern "C" {
#endif

struct log_interface {
	void (*puts)(const char *str);
};

struct log_provider {
	struct service_provider base_provider;
	struct log_interface *interface;
};

struct rpc_interface *log_provider_init(struct log_provider *context,
					struct log_interface *interface);

#ifdef __cplusplus
}
#endif

#endif /* LOG_PROVIDER_H */

/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SFS_HANDLERS_H
#define SFS_HANDLERS_H

#include <components/service/common/provider/service_provider.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sfs_provider {
	struct service_provider base_provider;
};

struct call_ep *sfs_provider_init(struct sfs_provider *context);
rpc_status_t sfs_set_handler(void *context, struct call_req *req);
rpc_status_t sfs_get_handler(void *context, struct call_req *req);
rpc_status_t sfs_get_info_handler(void *context, struct call_req *req);
rpc_status_t sfs_remove_handler(void *context, struct call_req *req);

#ifdef __cplusplus
}
#endif

#endif /* SFS_HANDLERS_H */

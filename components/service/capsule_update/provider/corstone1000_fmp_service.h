/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORSTONE1000_FMP_SERVICE_H
#define CORSTONE1000_FMP_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rpc_caller.h>
#include <psa/client.h>

void provision_fmp_variables_metadata(struct rpc_caller *caller);

void set_fmp_image_info(struct rpc_caller *caller,
		psa_handle_t platform_service_handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CORSTONE1000_FMP_SERVICE_H */

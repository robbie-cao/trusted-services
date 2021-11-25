/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPC_STATUS_H
#define RPC_STATUS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief RPC status code type
 *
 * Used for returning the status of an RPC transaction.  This is
 * different from the opstatus which is used to return an endpoint
 * specific status value.
 */
typedef int32_t rpc_status_t;

/** \brief RPC operation status code type
 *
 * Used for returning the endpoint specific operation status.
 * Different service layer protocols will use different status
 * value schemes. Status values returned by an operation are
 * carried by the RPC layer using this type.
 */
typedef int64_t rpc_opstatus_t;

#ifdef __cplusplus
}
#endif

#endif /* RPC_STATUS_H */

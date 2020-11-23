/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PROTOCOLS_RPC_COMMON_STATUS_H
#define PROTOCOLS_RPC_COMMON_STATUS_H

/* Common RPC status codes for C/C++
 *
 * Alignment of these definitions with other defintions for
 * alternative languages is checked through a set of test cases.
 * These status values are aligned to PSA definitions.
 */
#define TS_RPC_CALL_ACCEPTED                            (0)
#define TS_RPC_ERROR_EP_DOES_NOT_EXIT                   (-1)
#define TS_RPC_ERROR_INVALID_OPCODE                     (-2)
#define TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED        (-3)
#define TS_RPC_ERROR_INVALID_REQ_BODY                   (-4)
#define TS_RPC_ERROR_INVALID_RESP_BODY                  (-5)
#define TS_RPC_ERROR_RESOURCE_FAILURE		            (-6)
#define TS_RPC_ERROR_NOT_READY		                    (-7)
#define TS_RPC_ERROR_INVALID_TRANSACTION		        (-8)
#define TS_RPC_ERROR_INTERNAL		                    (-9)
#define TS_RPC_ERROR_INVALID_PARAMETER		            (-10)

#endif /* PROTOCOLS_RPC_COMMON_STATUS_H */

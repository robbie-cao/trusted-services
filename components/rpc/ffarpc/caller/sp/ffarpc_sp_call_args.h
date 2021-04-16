/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef FFA_RPC_SP_CALL_ARGS_H_
#define FFA_RPC_SP_CALL_ARGS_H_

#include "components/rpc/ffarpc/endpoint/ffarpc_call_args.h"

/*
 * sp_msg args is shifted by one compared to ffa_direct_msg because the first
 * parameter register (w3/w3) is used by the routing extension and it is not
 * included in sp_msg args.
 */
#define FFA_TO_SP_CALL_OFFSET(offset) ((offset)-1)

/* Common req & resp arg offests into sp_msg args structure */
#define SP_CALL_ARGS_IFACE_ID_OPCODE                                           \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_IFACE_ID_OPCODE)

/* Req arg offsets */
#define SP_CALL_ARGS_REQ_DATA_LEN                                              \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_REQ_DATA_LEN)
#define SP_CALL_ARGS_CALLER_ID FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_CALLER_ID)
#define SP_CALL_ARGS_ENCODING FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_ENCODING)

/* Resp arg offsets */
#define SP_CALL_ARGS_RESP_DATA_LEN                                             \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_RESP_DATA_LEN)
#define SP_CALL_ARGS_RESP_RPC_STATUS                                           \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_RESP_RPC_STATUS)
#define SP_CALL_ARGS_RESP_OP_STATUS                                            \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_RESP_OP_STATUS)

/* Share/unshare offsets */
#define SP_CALL_ARGS_SHARE_MEM_HANDLE_LSW                                      \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW)
#define SP_CALL_ARGS_SHARE_MEM_HANDLE_MSW                                      \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW)
#define SP_CALL_ARGS_SHARE_MEM_SIZE                                            \
	FFA_TO_SP_CALL_OFFSET(FFA_CALL_ARGS_SHARE_MEM_SIZE)

#endif /* FFA_RPC_SP_CALL_ARGS_H_ */

/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FFA_CALL_ARGS_H
#define FFA_CALL_ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Defines convention for use of FFA direct call arguments by
 * the SP service framework. This header file is used by the
 * normal world RPC caller and the RPC listener in the SP.
 */

/* Macros for parameters carried in a single register */
#define FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(i, o) \
    (((i) << 16) | ((o) & 0xffff))
#define FFA_CALL_ARGS_EXTRACT_IFACE(reg) \
    ((reg) >> 16)
#define FFA_CALL_ARGS_EXTRACT_OPCODE(reg) \
    ((reg) & 0xffff)

/* Common req & resp arg offests into msg_args structure */
#define FFA_CALL_ARGS_IFACE_ID_OPCODE	    (0)

/* Req arg offsets */
#define FFA_CALL_ARGS_REQ_DATA_LEN		    (1)
#define FFA_CALL_ARGS_CALLER_ID		        (2)
#define FFA_CALL_ARGS_ENCODING		        (3)

/* Resp arg offsets */
#define FFA_CALL_ARGS_RESP_DATA_LEN		    (1)
#define FFA_CALL_ARGS_RESP_RPC_STATUS	    (2)
#define FFA_CALL_ARGS_RESP_OP_STATUS		(3)

/* Share/unshare offsets */
#define FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW	(1)
#define FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW	(2)
#define FFA_CALL_ARGS_SHARE_MEM_SIZE		(3)

#ifdef __cplusplus
}
#endif

#endif /* FFA_CALL_ARGS_H */

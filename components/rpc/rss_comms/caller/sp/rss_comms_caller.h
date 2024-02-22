/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RSS_COMMS_CALLER_H__
#define __RSS_COMMS_CALLER_H__

#include "rpc_caller.h"
#include "rss_comms_messenger_api.h"

typedef void *rss_comms_call_handle;

rpc_status_t rss_comms_caller_init(struct rpc_caller_interface *rpc_caller);
rpc_status_t rss_comms_caller_deinit(struct rpc_caller_interface *rpc_caller);

rss_comms_call_handle rss_comms_caller_begin(struct rpc_caller_interface *caller,
					     uint8_t **request_buffer, size_t request_length);

rpc_status_t rss_comms_caller_invoke(rss_comms_call_handle handle, uint32_t opcode,
				     uint8_t **response_buffer, size_t *response_length);

rpc_status_t rss_comms_caller_end(rss_comms_call_handle handle);

#endif /* __RSS_COMMS_CALLER_H__ */

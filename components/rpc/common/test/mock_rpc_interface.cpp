// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <CppUTestExt/MockSupport.h>
#include "mock_rpc_interface.h"
#include "call_req_comparator.h"

static call_req_comparator req_comparator(call_req_comparator::mode_ignore_opstatus);

void mock_rpc_interface_init(void)
{
	mock().installComparator("call_req", req_comparator);
}

void expect_mock_rpc_interface_receive(struct rpc_interface *iface,
				       const struct call_req *req, rpc_status_t result)
{
	mock().expectOneCall("rpc_interface_receive").
		onObject(iface).
		withOutputParameterReturning("opstatus", &req->opstatus, sizeof(req->opstatus)).
		withOutputParameterReturning("resp_buf_data_len", &req->resp_buf.data_len,
					     sizeof(req->resp_buf.data_len)).
		withParameterOfType("call_req", "req", req).
		andReturnValue(result);
}

rpc_status_t mock_rpc_interface_receive(struct rpc_interface *iface,
					struct call_req *req)
{
	return mock().actualCall("rpc_interface_receive").
		onObject(iface).
		withOutputParameter("opstatus", &req->opstatus).
		withOutputParameter("resp_buf_data_len", &req->resp_buf.data_len).
		withParameterOfType("call_req", "req", req).
		returnIntValue();
}

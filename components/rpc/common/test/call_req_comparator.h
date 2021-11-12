/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#ifndef CALL_REQ_COMPARATOR_H_
#define CALL_REQ_COMPARATOR_H_

#include <CppUTestExt/MockSupport.h>
#include <inttypes.h>
#include "call_param_buf_comparator.h"

class call_req_comparator : public MockNamedValueComparator
{
public:
	enum check_mode {
		mode_normal = 0,
		mode_ignore_opstatus
	};

	call_req_comparator(check_mode mode) : mode(mode)
	{
	}

	virtual bool isEqual(const void *object1, const void *object2)
	{
		struct call_req *req1 = (struct call_req *)object1;
		struct call_req *req2 = (struct call_req *)object2;
		call_param_buf_comparator buf_comparator_normal;
		call_param_buf_comparator buf_comparator_ignore_data_len(
			call_param_buf_comparator::mode_ignore_data_len);

		return (req1->caller_id == req2->caller_id) &&
			(req1->interface_id == req2->interface_id) &&
			(req1->opcode == req2->opcode) &&
			(req1->encoding == req2->encoding) &&
			(mode == mode_ignore_opstatus || req1->opstatus == req2->opstatus) &&
			buf_comparator_normal.isEqual(&req1->req_buf, &req2->req_buf) &&
			buf_comparator_ignore_data_len.isEqual(&req1->resp_buf, &req2->resp_buf);
	}

	// LCOV_EXCL_START
	virtual SimpleString valueToString(const void *object)
	{
		struct call_req *req = (struct call_req *)object;
		call_param_buf_comparator buf_comparator_normal;
		call_param_buf_comparator buf_comparator_ignore_data_len(
			call_param_buf_comparator::mode_ignore_data_len);
		SimpleString req_buf_str = buf_comparator_normal.valueToString(&req->req_buf);
		SimpleString resp_buf_str =
			buf_comparator_ignore_data_len.valueToString(&req->resp_buf);

		return StringFromFormat("caller_id = 0x%" PRIx32 ", interface_id = %" PRIu32 ", " \
					"opcode = %" PRIu32 ", encoding = %" PRIu32 ", " \
					"opstatus = 0x%" PRIx64 "%s, req_buf = %s, " \
					"resp_buf = %s",
					req->caller_id, req->interface_id, req->opcode,
					req->encoding, req->opstatus,
					(mode == mode_ignore_opstatus) ? " (ignore)" : "",
					req_buf_str.asCharString(), resp_buf_str.asCharString());
	}
	// LCOV_EXCL_STOP

private:
	check_mode mode;
};

#endif /* CALL_REQ_COMPARATOR_H_ */

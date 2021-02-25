// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include "ffa_api.h"
#include "sp_api_defines.h"
#include "sp_messaging.h"
#if FFA_DIRECT_MSG_ROUTING_EXTENSION
#include "ffa_direct_msg_routing_extension.h"
#endif

#include <string.h>

#define SP_MSG_ARG_OFFSET (1)

static void pack_ffa_direct_msg(const struct sp_msg *msg,
				struct ffa_direct_msg *ffa_msg)
{
	uint32_t i = 0;

	ffa_msg->source_id = msg->source_id;
	ffa_msg->destination_id = msg->destination_id;

	ffa_msg->args[0] = 0;
	memcpy(&ffa_msg->args[SP_MSG_ARG_OFFSET], msg->args, sizeof(msg->args));
}

static void unpack_ffa_direct_msg(const struct ffa_direct_msg *ffa_msg,
				  struct sp_msg *msg)
{
	uint32_t i = 0;

	if (ffa_msg->function_id != FFA_SUCCESS_32) {
		/*
		 * Handling request or response (error is handled before call)
		 */
		msg->source_id = ffa_msg->source_id;
		msg->destination_id = ffa_msg->destination_id;

		memcpy(msg->args, &ffa_msg->args[SP_MSG_ARG_OFFSET],
		       sizeof(msg->args));
	} else {
		/* Success has no message parameters */
		*msg = (struct sp_msg){ 0 };
	}
}

sp_result sp_msg_wait(struct sp_msg *msg)
{
	ffa_result ffa_res = FFA_OK;
	struct ffa_direct_msg ffa_msg = { 0 };

	if (!msg)
		return SP_RESULT_INVALID_PARAMETERS;

	ffa_res = ffa_msg_wait(&ffa_msg);
	if (ffa_res != FFA_OK) {
		*msg = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
	ffa_res = ffa_direct_msg_routing_ext_wait_post_hook(&ffa_msg);
	if (ffa_res != FFA_OK) {
		*msg = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}
#endif

	unpack_ffa_direct_msg(&ffa_msg, msg);

	return SP_RESULT_OK;
}

sp_result sp_msg_send_direct_req(const struct sp_msg *req, struct sp_msg *resp)
{
	ffa_result ffa_res = FFA_OK;
	struct ffa_direct_msg ffa_req = { 0 };
	struct ffa_direct_msg ffa_resp = { 0 };

	if (!resp)
		return SP_RESULT_INVALID_PARAMETERS;

	if (!req) {
		*resp = (struct sp_msg){ 0 };
		return SP_RESULT_INVALID_PARAMETERS;
	}

	pack_ffa_direct_msg(req, &ffa_req);

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
	ffa_direct_msg_routing_ext_req_pre_hook(&ffa_req);
#endif

	ffa_res = ffa_msg_send_direct_req(ffa_req.source_id,
					  ffa_req.destination_id,
					  ffa_req.args[0], ffa_req.args[1],
					  ffa_req.args[2], ffa_req.args[3],
					  ffa_req.args[4], &ffa_resp);

	if (ffa_res != FFA_OK) {
#if FFA_DIRECT_MSG_ROUTING_EXTENSION
		ffa_direct_msg_routing_ext_req_error_hook();
#endif
		*resp = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
	ffa_res = ffa_direct_msg_routing_ext_req_post_hook(&ffa_resp);
	if (ffa_res != SP_RESULT_OK) {
		*resp = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}
#endif

	unpack_ffa_direct_msg(&ffa_resp, resp);

	return SP_RESULT_OK;
}

sp_result sp_msg_send_direct_resp(const struct sp_msg *resp, struct sp_msg *req)
{
	ffa_result ffa_res = FFA_OK;
	struct ffa_direct_msg ffa_resp = { 0 };
	struct ffa_direct_msg ffa_req = { 0 };

	if (!req)
		return SP_RESULT_INVALID_PARAMETERS;

	if (!resp) {
		*req = (struct sp_msg){ 0 };
		return SP_RESULT_INVALID_PARAMETERS;
	}

	pack_ffa_direct_msg(resp, &ffa_resp);

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
	ffa_direct_msg_routing_ext_resp_pre_hook(&ffa_resp);
#endif

	ffa_res = ffa_msg_send_direct_resp(ffa_resp.source_id,
					   ffa_resp.destination_id,
					   ffa_resp.args[0], ffa_resp.args[1],
					   ffa_resp.args[2], ffa_resp.args[3],
					   ffa_resp.args[4], &ffa_req);

	if (ffa_res != FFA_OK) {
#if FFA_DIRECT_MSG_ROUTING_EXTENSION
		ffa_direct_msg_routing_ext_resp_error_hook();
#endif
		*req = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
	ffa_res = ffa_direct_msg_routing_ext_resp_post_hook(&ffa_req);
	if (ffa_res != SP_RESULT_OK) {
		*req = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}
#endif

	unpack_ffa_direct_msg(&ffa_req, req);

	return SP_RESULT_OK;
}

#if FFA_DIRECT_MSG_ROUTING_EXTENSION
sp_result sp_msg_send_rc_req(const struct sp_msg *req, struct sp_msg *resp)
{
	ffa_result ffa_res = FFA_OK;
	struct ffa_direct_msg ffa_req = { 0 };
	struct ffa_direct_msg ffa_resp = { 0 };

	if (!resp)
		return SP_RESULT_INVALID_PARAMETERS;

	if (!req) {
		*resp = (struct sp_msg){ 0 };
		return SP_RESULT_INVALID_PARAMETERS;
	}

	pack_ffa_direct_msg(req, &ffa_req);

	ffa_direct_msg_routing_ext_rc_req_pre_hook(&ffa_req);

	ffa_res = ffa_msg_send_direct_resp(ffa_req.source_id,
					   ffa_req.destination_id,
					   ffa_req.args[0], ffa_req.args[1],
					   ffa_req.args[2], ffa_req.args[3],
					   ffa_req.args[4], &ffa_resp);

	if (ffa_res != FFA_OK) {
		ffa_direct_msg_routing_ext_rc_req_error_hook();
		*resp = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}

	ffa_res = ffa_direct_msg_routing_ext_rc_req_post_hook(&ffa_resp);
	if (ffa_res != SP_RESULT_OK) {
		*resp = (struct sp_msg){ 0 };
		return SP_RESULT_FFA(ffa_res);
	}

	unpack_ffa_direct_msg(&ffa_resp, resp);

	return SP_RESULT_OK;
}
#endif

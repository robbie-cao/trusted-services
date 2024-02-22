/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <trace.h>

#include "protocols/rpc/common/packed-c/status.h"
#include "rss_comms_messenger_api.h"
#include "rss_comms_platform_api.h"

struct rss_comms_msg {
	uint8_t *req_buf;
	size_t req_len;
	uint8_t *resp_buf;
};

int rss_comms_messenger_init(struct rss_comms_messenger *rss_comms)
{
	int ret = 0;

	if (!rss_comms || rss_comms->msg || rss_comms->platform)
		return -1;

	rss_comms->msg = calloc(1, sizeof(struct rss_comms_msg));
	if (!rss_comms->msg)
		return -1;

	rss_comms->platform = rss_comms_platform_init();
	if (!rss_comms->platform) {
		EMSG("Platform init failed");
		free(rss_comms->msg);
		rss_comms->msg = NULL;
		return ret;
	}

	return 0;
}

void rss_comms_messenger_deinit(struct rss_comms_messenger *rss_comms)
{
	struct rss_comms_msg *msg = NULL;

	if (!rss_comms)
		return;

	if (rss_comms->msg) {
		msg = (struct rss_comms_msg *)rss_comms->msg;

		if (msg->req_buf)
			free(msg->req_buf);
		if (msg->resp_buf)
			free(msg->resp_buf);

		free(msg);
		rss_comms->msg = NULL;
	}

	rss_comms_platform_deinit(rss_comms->platform);
	rss_comms->platform = NULL;
}

int rss_comms_messenger_call_invoke(struct rss_comms_messenger *rss_comms, uint8_t **resp_buf,
				    size_t *resp_len)
{
	struct rss_comms_msg *msg = NULL;
	int ret = 0;

	if (!rss_comms || !resp_buf || !resp_len) {
		EMSG("Invalid arguments");
		return -1;
	}

	if (!rss_comms->msg) {
		EMSG("msg is null");
		return -1;
	}

	msg = (struct rss_comms_msg *)rss_comms->msg;
	*resp_buf = calloc(1, *resp_len);

	if (!(*resp_buf))
		return -1;

	ret = rss_comms_platform_invoke(rss_comms->platform, *resp_buf, msg->req_buf, resp_len,
					msg->req_len);

	if (ret < 0) {
		free(*resp_buf);
		*resp_buf = NULL;
		return ret;
	}

	msg->resp_buf = *resp_buf;

	return 0;
}

int rss_comms_messenger_call_begin(struct rss_comms_messenger *rss_comms, uint8_t **req_buf,
				   size_t req_len)
{
	int ret = 0;
	struct rss_comms_msg *msg = NULL;

	if (!rss_comms || !req_buf || !rss_comms->msg)
		return -1;

	if (req_len > UINT32_MAX || req_len == 0) {
		EMSG("req_len invalid: %lu", req_len);
		return -1;
	}

	msg = (struct rss_comms_msg *)rss_comms->msg;

	if (msg->req_buf)
		return -1;

	msg->req_buf = calloc(1, req_len);

	if (!msg->req_buf)
		return -1;

	*req_buf = msg->req_buf;
	msg->req_len = req_len;

	ret = rss_comms_platform_begin(rss_comms->platform, *req_buf, req_len);

	return ret;
}

void rss_comms_messenger_call_end(struct rss_comms_messenger *rss_comms)
{
	int ret = 0;
	struct rss_comms_msg *msg = NULL;

	if (!rss_comms || !rss_comms->msg)
		return;

	msg = (struct rss_comms_msg *)rss_comms->msg;

	if (msg->req_buf)
		free(msg->req_buf);

	if (msg->resp_buf)
		free(msg->resp_buf);

	msg->req_len = 0;
	msg->req_buf = NULL;
	msg->resp_buf = NULL;

	ret = rss_comms_platform_end(rss_comms->platform);

	if (ret < 0) {
		EMSG("Platform end failed: %d", ret);
		return;
	}
}

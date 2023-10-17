/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <trace.h>
#include "rss_comms_caller.h"
#include "rss_comms_mhu.h"
#include <protocols/rpc/common/packed-c/status.h>
#include <platform/drivers/arm/mhu_driver/mhu_v3_x.h>

static rpc_status_t rss_comms_call_invoke(void *context, rpc_call_handle handle,
                                          uint32_t opcode, int *opstatus,
                                          uint8_t **resp_buf, size_t *resp_len)
{
	struct rss_comms_caller *rss_comms = context;
	struct mhu_v3_x_dev_t *rx_dev;
	struct mhu_v3_x_dev_t *tx_dev;
	struct rss_comms_mhu *mhu;
	struct rss_comms_shm *rss_shm;
	union rss_comms_io_buffer_t io_buf;
	struct rss_comms_messenger *msg;
	enum mhu_error_t err;

	(void)opcode;

	if ((handle != rss_comms) || !opstatus || !resp_buf || !resp_len || !rss_comms) {
		EMSG("[RSS-COMMS]: call_invoke: invalid arguments\n");
		return TS_RPC_ERROR_INVALID_PARAMETER;
	}

	msg = &rss_comms->rss_comms_msg;
	if (!msg->transport) {
		EMSG("[RSS-COMMS] transport is null\n");
		return PSA_ERROR_GENERIC_ERROR;
	}

	mhu = msg->transport;
	rx_dev = &mhu->rx_dev;
	tx_dev = &mhu->tx_dev;
	rss_shm = msg->platform;
	io_buf = rss_shm->io_buf;

	err = mhu_send_data(tx_dev, rss_shm->shm_addr, (uint8_t *)&io_buf.msg, rss_shm->msg_size);
	if (err != MHU_ERR_NONE) {
		EMSG("[RSS-COMMS] mhu send data failed!\n");
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

#if DEBUG
	/*
	 * Poisoning the message buffer (with a known pattern).
	 * Helps in detecting hypothetical RSS communication bugs.
	 */
	memset(&io_buf.msg, 0xA5, msg_size);
#endif

	err = mhu_receive_data(rx_dev, rss_shm->shm_addr, (uint8_t *)&io_buf.reply, resp_len);
	if (err != MHU_ERR_NONE) {
		EMSG("[RSS-COMMS] mhu receive data failed!\n");
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

	*resp_buf = &io_buf.reply;
	*opstatus = 0;

	return TS_RPC_CALL_ACCEPTED;
}

struct rpc_caller *rss_comms_caller_init(struct rss_comms_caller *rss_comms)
{
	struct rpc_caller *rpc = &rss_comms->rpc_caller;
	int ret;

	if (!rss_comms) {
		EMSG("[RSS-COMMS] caller: invalid argument\n");
		return NULL;
	}

	ret = rss_comms_messenger_init(&rss_comms->rss_comms_msg);
	if (ret < 0) {
		EMSG("[RSS-COMMS] mssenger init failed\n");
		return NULL;
	}

	rpc_caller_init(rpc, rss_comms);

	rpc->call_begin = NULL;
	rpc->call_invoke = rss_comms_call_invoke;
	rpc->call_end = NULL;

	return rpc;
}

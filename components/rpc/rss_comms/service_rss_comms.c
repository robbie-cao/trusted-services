/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2023, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>
#include <platform/drivers/arm/mhu_driver/mhu_v3_x.h>
#include <psa/client.h>
#include <rss_comms_protocol.h>
#include "rss_comms_caller.h"
#include "trace.h"
#include "rss_comms_messenger_api.h"
#include <protocols/rpc/common/packed-c/status.h>

#define AP_RSS_SND_MHU_BASE    0x2AB20000
#define AP_RSS_RCV_MHU_BASE    0x2AB30000

extern size_t mhu_get_max_message_size(void);

static uint8_t select_protocol_version(const psa_invec *in_vec, size_t in_len,
				       const psa_outvec *out_vec, size_t out_len)
{
	size_t comms_mhu_msg_size;
	size_t comms_embed_msg_min_size;
	size_t comms_embed_reply_min_size;
	size_t in_size_total = 0;
	size_t out_size_total = 0;
	size_t i;

	for (i = 0U; i < in_len; ++i) {
		in_size_total += in_vec[i].len;
	}
	for (i = 0U; i < out_len; ++i) {
		out_size_total += out_vec[i].len;
	}

	comms_mhu_msg_size = mhu_get_max_message_size();

	comms_embed_msg_min_size = sizeof(struct serialized_rss_comms_header_t) +
				   sizeof(struct rss_embed_msg_t) -
				   PLAT_RSS_COMMS_PAYLOAD_MAX_SIZE;

	comms_embed_reply_min_size = sizeof(struct serialized_rss_comms_header_t) +
				     sizeof(struct rss_embed_reply_t) -
				     PLAT_RSS_COMMS_PAYLOAD_MAX_SIZE;

	/* Use embed if we can pack into one message and reply, else use
	 * pointer_access. The underlying MHU transport protocol uses a
	 * single uint32_t to track the length, so the amount of data that
	 * can be in a message is 4 bytes less than mhu_get_max_message_size
	 * reports.
	 *
	 * TODO tune this with real performance numbers, it's possible a
	 * pointer_access message is less performant than multiple embed
	 * messages due to ATU configuration costs to allow access to the
	 * pointers.
	 */
	if ((comms_embed_msg_min_size + in_size_total > comms_mhu_msg_size - sizeof(uint32_t))
	 || (comms_embed_reply_min_size + out_size_total > comms_mhu_msg_size - sizeof(uint32_t))) {
		return RSS_COMMS_PROTOCOL_POINTER_ACCESS;
	} else {
		return RSS_COMMS_PROTOCOL_EMBED;
	}
}

psa_status_t __psa_call(struct rpc_caller *caller, psa_handle_t handle,
		               int32_t client_id, int32_t type,
					   const struct psa_invec *in_vec, size_t in_len,
		               struct psa_outvec *out_vec, size_t out_len)
{
	/* Declared statically to avoid using huge amounts of stack space. Maybe revisit if
	 * functions not being reentrant becomes a problem.
	 */
	union rss_comms_io_buffer_t io_buf;
	struct rss_comms_messenger *rpmsg;
	psa_status_t psa_status = PSA_SUCCESS;
	static uint8_t seq_num = 1U;
	psa_status_t status;
	size_t reply_size = sizeof(io_buf.reply);
	psa_status_t return_val;
	struct rss_comms_virtio *virtio;
	struct rss_comms_caller *rss_comms;
	rpc_call_handle rpc_handle;
	rpc_handle = caller;
	size_t msg_size = 0;
	int ret = 0;
	uint8_t *resp;

	if (!caller) {
		EMSG("[RSS-COMMS] psa_call rpc caller is null\n");
		return PSA_ERROR_GENERIC_ERROR;
	}

	rss_comms = caller->context;
	if( !rss_comms ) {
		EMSG("[RSS-COMMS] psa_call rss comms is null\n");
		return PSA_ERROR_GENERIC_ERROR;
	}

	rpmsg = &rss_comms->rss_comms_msg;
	if (!rpmsg->transport) {
		EMSG("[RSS-COMMS] psa_call messenger is null\n");
		return PSA_ERROR_GENERIC_ERROR;
	}

	virtio = (struct rss_comms_virtio *)rpmsg->platform;

	if (type > INT16_MAX || type < INT16_MIN || in_len > PSA_MAX_IOVEC
	    || out_len > PSA_MAX_IOVEC) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	virtio->io_buf.msg.header.seq_num = seq_num;
	/* No need to distinguish callers (currently concurrent calls are not supported). */
	virtio->io_buf.msg.header.client_id = client_id;
	virtio->io_buf.msg.header.protocol_ver = select_protocol_version(in_vec, in_len, out_vec, out_len);

	status = rss_protocol_serialize_msg(handle, type, in_vec, in_len, out_vec,
					    out_len, &virtio->io_buf.msg, &msg_size);
	if (status != PSA_SUCCESS) {
		EMSG("psa_call: serialize msg failed: %d\n", status);
		return status;
	}

	virtio->msg_size = msg_size;

	IMSG("[RSS-COMMS] Sending message\n");
	IMSG("protocol_ver=%u\n", virtio->io_buf.msg.header.protocol_ver);
	IMSG("seq_num=%u\n", virtio->io_buf.msg.header.seq_num);
	IMSG("client_id=%u\n", virtio->io_buf.msg.header.client_id);

	ret = rpc_caller_invoke(caller, rpc_handle, 0, &psa_status, &resp,  &reply_size);
	if (ret != TS_RPC_CALL_ACCEPTED) {
		EMSG("psa_call: invoke failed: %d\n", ret);
		return PSA_ERROR_GENERIC_ERROR;
	}

	virtio->io_buf.reply = *(struct serialized_rss_comms_reply_t *)resp;

	IMSG("[RSS-COMMS] Received reply\n");
	IMSG("protocol_ver=%u\n", virtio->io_buf.reply.header.protocol_ver);
	IMSG("seq_num=%u\n", virtio->io_buf.reply.header.seq_num);
	IMSG("client_id=%u\n", virtio->io_buf.reply.header.client_id);
	IMSG("reply_size=%lu\n", reply_size);

	status = rss_protocol_deserialize_reply(out_vec, out_len, &return_val,
						&virtio->io_buf.reply, reply_size);
	if (status != PSA_SUCCESS) {
		EMSG("psa_call: protocol deserialize reply failed: %d\n", status);
		return status;
	}

	IMSG("return_val=%d\n", return_val);

	/* Clear the MHU message buffer to remove assets from memory */
	memset(&virtio->io_buf, 0x0, sizeof(virtio->io_buf));

	seq_num++;

	return return_val;
}

psa_status_t psa_call_client_id(struct rpc_caller *caller,
				psa_handle_t psa_handle, int32_t client_id,
				int32_t type, const struct psa_invec *in_vec,
				size_t in_len, struct psa_outvec *out_vec,
				size_t out_len)
{
	return __psa_call(caller, psa_handle, client_id, type, in_vec, in_len,
			  out_vec, out_len);
}

psa_status_t psa_call(struct rpc_caller *caller, psa_handle_t psa_handle,
		      int32_t type, const struct psa_invec *in_vec,
		      size_t in_len, struct psa_outvec *out_vec, size_t out_len)
{
	return __psa_call(caller, psa_handle, 0, type, in_vec, in_len, out_vec,
			  out_len);
}

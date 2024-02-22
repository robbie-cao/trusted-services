/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_COMMS_PROTOCOL_EMBED_H__
#define __RSS_COMMS_PROTOCOL_EMBED_H__

#include <psa/client.h>
#include <sys/cdefs.h>

struct __packed rss_embed_msg_t {
	psa_handle_t handle;
	uint32_t ctrl_param; /* type, in_len, out_len */
	uint16_t io_size[PSA_MAX_IOVEC];
	uint8_t trailer[PLAT_RSS_COMMS_PAYLOAD_MAX_SIZE];
};

struct __packed rss_embed_reply_t {
	int32_t return_val;
	uint16_t out_size[PSA_MAX_IOVEC];
	uint8_t trailer[PLAT_RSS_COMMS_PAYLOAD_MAX_SIZE];
};

psa_status_t rss_protocol_embed_serialize_msg(psa_handle_t handle,
					      int16_t type,
					      const struct psa_invec *in_vec,
					      uint8_t in_len,
					      const struct psa_outvec *out_vec,
					      uint8_t out_len,
					      struct rss_embed_msg_t *msg,
					      size_t *msg_len);

psa_status_t rss_protocol_embed_deserialize_reply(struct psa_outvec *out_vec,
						  uint8_t out_len,
						  psa_status_t *return_val,
						  const struct rss_embed_reply_t *reply,
						  size_t reply_size);

psa_status_t rss_protocol_embed_calculate_msg_len(psa_handle_t handle,
						  const struct psa_invec *in_vec,
						  uint8_t in_len,
						  size_t *msg_len);

#endif /* __RSS_COMMS_PROTOCOL_EMBED_H__ */

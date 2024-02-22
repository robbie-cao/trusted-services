/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_COMMS_PROTOCOL_H__
#define __RSS_COMMS_PROTOCOL_H__

#include <stdint.h>

#include <psa/client.h>
#include "rss_comms_protocol_embed.h"
#include "rss_comms_protocol_pointer_access.h"
#include "service/common/include/psa/client.h"

enum rss_comms_protocol_version_t {
	RSS_COMMS_PROTOCOL_EMBED = 0,
	RSS_COMMS_PROTOCOL_POINTER_ACCESS = 1,
};

struct __packed serialized_rss_comms_header_t {
	uint8_t protocol_ver;
	uint8_t seq_num;
	uint16_t client_id;
};

/* MHU message passed from Host to RSS to deliver a PSA client call */
struct __packed serialized_rss_comms_msg_t {
	struct serialized_rss_comms_header_t header;
	union __packed {
		struct rss_embed_msg_t embed;
		struct rss_pointer_access_msg_t pointer_access;
	} msg;
};

/* MHU reply message to hold the PSA client reply result returned by RSS */
struct __packed serialized_rss_comms_reply_t {
	struct serialized_rss_comms_header_t header;
	union __packed {
		struct rss_embed_reply_t embed;
		struct rss_pointer_access_reply_t pointer_access;
	} reply;
};

psa_status_t rss_protocol_serialize_msg(psa_handle_t handle,
					int16_t type,
					const struct psa_invec *in_vec,
					uint8_t in_len,
					const struct psa_outvec *out_vec,
					uint8_t out_len,
					struct serialized_rss_comms_msg_t *msg,
					size_t *msg_len);

psa_status_t rss_protocol_deserialize_reply(struct psa_outvec *out_vec,
					    uint8_t out_len,
					    psa_status_t *return_val,
					    const struct serialized_rss_comms_reply_t *reply,
					    size_t reply_size);

psa_status_t rss_protocol_calculate_msg_len(psa_handle_t handle,
					    uint8_t protocol_ver,
					    const struct psa_invec *in_vec,
					    uint8_t in_len,
					    size_t *msg_len);

#endif /* __RSS_COMMS_PROTOCOL_H__ */

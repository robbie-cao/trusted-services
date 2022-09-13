/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

int ffa_tee_find_dev(char *path, size_t path_len);
int ffa_tee_open_session(int fd, uint16_t part_id);
void ffa_tee_close_session(int fd);
int ffa_tee_send_msg(int fd, uint16_t iface_id, uint16_t opcode, size_t req_len, uint16_t encoding,
		     size_t *resp_len, int32_t *rpc_status, int32_t *opstatus);
int ffa_tee_list_part_ids(int fd, const uint8_t *uuid, uint16_t *id_buf, size_t id_buf_cnt,
			  size_t *populated, bool *buf_short);
int ffa_tee_share_mem(int fd, size_t req_size, void **addr, size_t *size, int *id);
void ffa_tee_reclaim_mem(int fd, void *addr, size_t size);

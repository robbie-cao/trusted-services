/*
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffarpc_caller.h"
#include "ffa_tee.h"
#include <rpc/ffarpc/endpoint/ffarpc_call_args.h>
#include <rpc/ffarpc/endpoint/ffarpc_call_ops.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

#define KERNEL_MOD_REQ_VER_MAJOR 1
#define KERNEL_MOD_REQ_VER_MINOR 0
#define KERNEL_MOD_REQ_VER_PATCH 0

#define DEFAULT_SHMEM_BUF_SIZE			(4096)

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len);
static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
			rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len);
static void call_end(void *context, rpc_call_handle handle);

bool ffarpc_caller_check_version(void)
{
	FILE *f = NULL;
	int ver_major = -1, ver_minor = -1, ver_patch = -1, scan_cnt = 0;

	f = fopen("/sys/module/arm_ffa_tee/version", "r");
	if (f) {
		scan_cnt = fscanf(f, "%d.%d.%d", &ver_major, &ver_minor, &ver_patch);
		fclose(f);
		if (scan_cnt != 3) {
			printf("error: cannot read FF-A TEE driver version\n");
			return false;
		}
	} else {
		printf("error: FF-A TEE driver not available\n");
		return false;
	}

	if (ver_major != KERNEL_MOD_REQ_VER_MAJOR)
		goto err;

	if (ver_minor < KERNEL_MOD_REQ_VER_MINOR)
		goto err;

	if (ver_minor == KERNEL_MOD_REQ_VER_MINOR)
		if (ver_patch < KERNEL_MOD_REQ_VER_PATCH)
			goto err;

	return true;

err:
	printf("error: FF-A TEE driver is v%d.%d.%d but required v%d.%d.%d\n",
		ver_major, ver_minor, ver_patch, KERNEL_MOD_REQ_VER_MAJOR,
		KERNEL_MOD_REQ_VER_MINOR, KERNEL_MOD_REQ_VER_PATCH);

	return false;
}

int ffarpc_caller_find_dev_path(char *path, size_t path_len)
{
	return ffa_tee_find_dev(path, path_len);
}

struct rpc_caller *ffarpc_caller_init(struct ffarpc_caller *s, const char *device_path)
{
	struct rpc_caller *base = &s->rpc_caller;

	rpc_caller_init(base, s);
	base->call_begin = call_begin;
	base->call_invoke = call_invoke;
	base->call_end = call_end;

	s->device_path = device_path;
	s->fd = -1;
	s->dest_partition_id = 0;
	s->dest_iface_id = 0;
	s->shared_mem_id = -1;
	s->shared_mem_required_size = DEFAULT_SHMEM_BUF_SIZE;
	s->shared_mem_actual_size = 0;
	s->shared_mem_buf = NULL;
	s->req_len = 0;
	s->resp_len = 0;
	s->is_call_transaction_in_progess = false;

	return base;
}

void ffarpc_caller_deinit(struct ffarpc_caller *s)
{
	s->rpc_caller.context = NULL;
	s->rpc_caller.call_begin = NULL;
	s->rpc_caller.call_invoke = NULL;
	s->rpc_caller.call_end = NULL;

	call_end(s, s);
}

size_t ffarpc_caller_discover(const struct ffarpc_caller *s, const struct uuid_canonical *uuid,
						uint16_t *partition_ids, size_t discover_limit)
{
	size_t discover_count = 0;
	bool buf_short = false;

	if (uuid && partition_ids && s->device_path) {
		int fd = -1;

		fd = open(s->device_path, O_RDWR);

		if (fd >= 0) {
			int ioctl_status = -1;
			struct uuid_octets uuid_buf = {0};

			uuid_parse_to_octets(uuid->characters, uuid_buf.octets, UUID_OCTETS_LEN);

			/* Ignore short buffer for now */
			ioctl_status = ffa_tee_list_part_ids(fd, uuid_buf.octets, partition_ids,
							     discover_limit, &discover_count,
							     &buf_short);

			close(fd);

			if (ioctl_status) {
				printf("%s():%d error: %d\n", __func__, __LINE__, ioctl_status);
				return 0;
			}
		}
	}

	return discover_count;
}

int ffarpc_caller_open(struct ffarpc_caller *s, uint16_t dest_partition_id, uint16_t dest_iface_id)
{
	int ioctl_status = -1;

	if (s->device_path) {

		s->fd = open(s->device_path, O_RDWR);

		if (s->fd >= 0) {
			ioctl_status = ffa_tee_open_session(s->fd, dest_partition_id);

			if (ioctl_status == 0) {
				/* Session successfully opened */
				s->dest_partition_id = dest_partition_id;
				s->dest_iface_id = dest_iface_id;
			} else {
				close(s->fd);
				s->fd = -1;
			}
		}
	}

	return ioctl_status;
}

int ffarpc_caller_close(struct ffarpc_caller *s)
{
	if (s->fd >= 0) {
		ffa_tee_close_session(s->fd);
		close(s->fd);
		s->fd = -1;
		s->dest_partition_id = 0;
	}

	return 0;
}

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len)
{
	rpc_call_handle handle = NULL;
	struct ffarpc_caller *s = (struct ffarpc_caller*)context;
	int rc = -1;

	if (!s->is_call_transaction_in_progess) {

		s->is_call_transaction_in_progess = true;
		handle = s;

		rc = ffa_tee_share_mem(s->fd, MAX(s->shared_mem_required_size, req_len),
				       &s->shared_mem_buf, &s->shared_mem_actual_size,
				       &s->shared_mem_id);

		if (rc == 0) {

			*req_buf = s->shared_mem_buf;
			s->req_len = req_len;
		}
		else {
			/* Failed to allocate req buffer */
			handle = NULL;
			s->is_call_transaction_in_progess = false;
		}
	}

	return handle;
}

static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
				rpc_opstatus_t *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct ffarpc_caller *s = (struct ffarpc_caller*)context;

	if ((handle == s) && s->is_call_transaction_in_progess) {
		int kernel_op_status = 0;

		kernel_op_status = ffa_tee_send_msg(s->fd, s->dest_iface_id, opcode, s->req_len,
						    s->rpc_caller.encoding, &s->resp_len,
						    &rpc_status, opstatus);

		if (kernel_op_status == 0) {
			*resp_len = s->resp_len;
			*resp_buf = s->shared_mem_buf;
		}
	}

	return rpc_status;
}

static void call_end(void *context, rpc_call_handle handle)
{
	struct ffarpc_caller *s = (struct ffarpc_caller*)context;

	if ((handle == s) && s->is_call_transaction_in_progess) {

		/* Call transaction complete so free resource */
		if (s->shared_mem_buf != NULL && s->shared_mem_actual_size != 0) {
			ffa_tee_reclaim_mem(s->fd, s->shared_mem_buf, s->shared_mem_actual_size);
			s->shared_mem_buf = NULL;
			s->shared_mem_actual_size = 0;
		}
		s->req_len = 0;
		s->resp_len = 0;

		s->is_call_transaction_in_progess = false;
	}
}

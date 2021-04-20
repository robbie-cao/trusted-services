/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffarpc_caller.h"
#include <arm_ffa_user.h>
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

#define KERNEL_MOD_REQ_VER_MAJOR 2
#define KERNEL_MOD_REQ_VER_MINOR 0
#define KERNEL_MOD_REQ_VER_PATCH 0

#define DEFAULT_SHMEM_BUF_SIZE			(4096)

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len);
static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
			int *opstatus, uint8_t **resp_buf, size_t *resp_len);
static void call_end(void *context, rpc_call_handle handle);

static int kernel_write_req_buf(struct ffarpc_caller *s);
static int kernel_read_resp_buf(struct ffarpc_caller *s);
static int share_mem_with_partition(struct ffarpc_caller *s);
static int unshare_mem_with_partition(struct ffarpc_caller *s);

bool ffarpc_caller_check_version(void)
{
	FILE *f;
	char mod_name[64];
	int ver_major, ver_minor, ver_patch;
	bool mod_loaded = false;

	f = fopen("/proc/modules", "r");
	if (!f) {
		printf("error: cannot open /proc/modules\n");
		return false;
	}

	while (fscanf(f, "%64s %*[^\n]\n", mod_name) != EOF) {
		if (!strcmp(mod_name, "arm_ffa_user")) {
			mod_loaded = true;
			break;
		}
	}

	fclose(f);

	if (!mod_loaded) {
		printf("error: kernel module not loaded\n");
		return false;
	}

	f = fopen("/sys/module/arm_ffa_user/version", "r");
	if (f) {
		fscanf(f, "%d.%d.%d", &ver_major, &ver_minor, &ver_patch);
		fclose(f);
	} else {
		/*
		 * Fallback for the initial release of the kernel module, where
		 * the version definition was missing.
		 */
		ver_major = 1;
		ver_minor = 0;
		ver_patch = 0;
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
	printf("error: kernel module is v%d.%d.%d but required v%d.%d.%d\n",
		ver_major, ver_minor, ver_patch, KERNEL_MOD_REQ_VER_MAJOR,
		KERNEL_MOD_REQ_VER_MINOR, KERNEL_MOD_REQ_VER_PATCH);

	return false;
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
	s->shared_mem_handle = 0;
	s->shared_mem_required_size = DEFAULT_SHMEM_BUF_SIZE;
	s->req_buf = NULL;
	s->req_len = 0;
	s->resp_buf = NULL;
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
	ffarpc_caller_close(s);
}

size_t ffarpc_caller_discover(const struct ffarpc_caller *s, const struct uuid_canonical *uuid,
						uint16_t *partition_ids, size_t discover_limit)
{
	size_t discover_count = 0;

	if (uuid && partition_ids && s->device_path) {
		int fd;

		fd = open(s->device_path, O_RDWR);

		if (fd >= 0) {
			int ioctl_status;
			struct ffa_ioctl_ep_desc discovered_partition;

			discovered_partition.uuid_ptr = (uintptr_t)&uuid->characters;
			discovered_partition.id = 0;

			ioctl_status = ioctl(fd, FFA_IOC_GET_PART_ID, &discovered_partition);

			if ((ioctl_status == 0) && (discover_count < discover_limit)) {
				partition_ids[discover_count] = discovered_partition.id;
				++discover_count;
			}

			close(fd);
		}
	}

	return discover_count;
}

int ffarpc_caller_open(struct ffarpc_caller *s, uint16_t dest_partition_id, uint16_t dest_iface_id)
{
	int ioctl_status = -1;
	struct ffa_ioctl_shm_desc shm_desc = {.dst_id = dest_partition_id};

	if (s->device_path) {

		s->fd = open(s->device_path, O_RDWR);

		if (s->fd >= 0) {
			/* Allocate resource for session */
			ioctl_status = ioctl(s->fd, FFA_IOC_SHM_INIT, &shm_desc);

			if (ioctl_status == 0) {
				/* Session successfully opened */
				s->dest_partition_id = dest_partition_id;
				s->dest_iface_id = dest_iface_id;
				s->shared_mem_handle = shm_desc.handle;
				ioctl_status = share_mem_with_partition(s);
			}

			if (ioctl_status != 0)  {
				/* Resource allocation or sharing error */
				close(s->fd);
				s->fd = -1;
			}
		}
	}

	return ioctl_status;
}

int ffarpc_caller_close(struct ffarpc_caller *s)
{
	int ioctl_status = -1;
	struct ffa_ioctl_shm_desc shm_desc = {
		.dst_id = s->dest_partition_id,
		.handle = s->shared_mem_handle,
	};

	if (s->fd >= 0) {

		unshare_mem_with_partition(s);
		ioctl_status = ioctl(s->fd, FFA_IOC_SHM_DEINIT, &shm_desc);
		close(s->fd);
		s->fd = -1;
		s->dest_partition_id = 0;
	}

	return ioctl_status;
}

static rpc_call_handle call_begin(void *context, uint8_t **req_buf, size_t req_len)
{
	rpc_call_handle handle = NULL;
	struct ffarpc_caller *s = (struct ffarpc_caller*)context;

	if (!s->is_call_transaction_in_progess) {

		s->is_call_transaction_in_progess = true;
		handle = s;

		if (req_len > 0) {

			s->req_buf = malloc(req_len);

			if (s->req_buf) {

				*req_buf = s->req_buf;
				s->req_len = req_len;
			}
			else {
				/* Failed to allocate req buffer */
				handle = NULL;
				s->is_call_transaction_in_progess = false;
			}
		}
		else {

			*req_buf = NULL;
			s->req_buf = NULL;
			s->req_len = req_len;
		}
	}

	return handle;
}

static rpc_status_t call_invoke(void *context, rpc_call_handle handle, uint32_t opcode,
				int *opstatus, uint8_t **resp_buf, size_t *resp_len)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	struct ffarpc_caller *s = (struct ffarpc_caller*)context;

	if ((handle == s) && s->is_call_transaction_in_progess) {
		int kernel_op_status = 0;

		if (s->req_len > 0) {
			kernel_op_status = kernel_write_req_buf(s);
		}

		if (kernel_op_status == 0) {
			/* Make direct call to send the request */
			struct ffa_ioctl_msg_args direct_msg;
			memset(&direct_msg, 0, sizeof(direct_msg));

			direct_msg.dst_id = s->dest_partition_id;
			direct_msg.args[FFA_CALL_ARGS_IFACE_ID_OPCODE] =
				FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(s->dest_partition_id, opcode);
			direct_msg.args[FFA_CALL_ARGS_REQ_DATA_LEN] = (uint64_t)s->req_len;
			direct_msg.args[FFA_CALL_ARGS_ENCODING] = s->rpc_caller.encoding;

			/* Initialise the caller ID.  Depending on the call path, this may
			* be overridden by a higher privilege execution level, based on its
			* perspective of the caller identity.
			*/
			direct_msg.args[FFA_CALL_ARGS_CALLER_ID] = 0;

			kernel_op_status = ioctl(s->fd, FFA_IOC_MSG_SEND, &direct_msg);

			if (kernel_op_status == 0) {
				/* Send completed normally - ffa return args in msg_args struct */
				s->resp_len = (size_t)direct_msg.args[FFA_CALL_ARGS_RESP_DATA_LEN];
				rpc_status = (int)direct_msg.args[FFA_CALL_ARGS_RESP_RPC_STATUS];
				*opstatus = (int)direct_msg.args[FFA_CALL_ARGS_RESP_OP_STATUS];

				if (s->resp_len > 0) {
					s->resp_buf = malloc(s->resp_len);

					if (s->resp_buf) {
						kernel_op_status = kernel_read_resp_buf(s);

						if (kernel_op_status != 0) {
							/* Failed to read response buffer */
							rpc_status = TS_RPC_ERROR_INTERNAL;
						}
					}
					else {
						/* Failed to allocate response buffer */
						s->resp_len = 0;
						rpc_status = TS_RPC_ERROR_INTERNAL;
					}
				}
				else {
					/* No response parameters */
					s->resp_buf = NULL;
				}

				*resp_len = s->resp_len;
				*resp_buf = s->resp_buf;
			}
		}
	}

	return rpc_status;
}

static void call_end(void *context, rpc_call_handle handle)
{
	struct ffarpc_caller *s = (struct ffarpc_caller*)context;

	if ((handle == s) && s->is_call_transaction_in_progess) {

		/* Call transaction complete so free resource */
		free(s->req_buf);
		s->req_buf = NULL;
		s->req_len = 0;

		free(s->resp_buf);
		s->resp_buf = NULL;
		s->resp_len = 0;

		s->is_call_transaction_in_progess = false;
	}
}

static int kernel_write_req_buf(struct ffarpc_caller *s) {

	int ioctl_status;
	struct ffa_ioctl_buf_desc req_descr;

	req_descr.buf_ptr = (uintptr_t)s->req_buf;
	req_descr.buf_len = s->req_len;
	req_descr.dst_id = s->dest_partition_id;
	ioctl_status = ioctl(s->fd, FFA_IOC_SHM_WRITE, &req_descr);

	return ioctl_status;
}


static int kernel_read_resp_buf(struct ffarpc_caller *s) {

	int ioctl_status;
	struct ffa_ioctl_buf_desc resp_descr;

	resp_descr.buf_ptr = (uintptr_t)s->resp_buf;
	resp_descr.buf_len = s->resp_len;
	resp_descr.dst_id = s->dest_partition_id;
	ioctl_status = ioctl(s->fd, FFA_IOC_SHM_READ, &resp_descr);

	return ioctl_status;
}

static int share_mem_with_partition(struct ffarpc_caller *s) {

	int ioctl_status;
	struct ffa_ioctl_msg_args direct_msg;
	memset(&direct_msg, 0, sizeof(direct_msg));

	direct_msg.dst_id = s->dest_partition_id;
	direct_msg.args[FFA_CALL_ARGS_IFACE_ID_OPCODE] =
		FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(FFA_CALL_MGMT_IFACE_ID, FFA_CALL_OPCODE_SHARE_BUF);
	direct_msg.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW] = (uint32_t)s->shared_mem_handle;
	direct_msg.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW] = (uint32_t)(s->shared_mem_handle >> 32);
	direct_msg.args[FFA_CALL_ARGS_SHARE_MEM_SIZE] = (uint64_t)s->shared_mem_required_size;

	ioctl_status = ioctl(s->fd, FFA_IOC_MSG_SEND, &direct_msg);

	return ioctl_status;
}

static int unshare_mem_with_partition(struct ffarpc_caller *s) {

	int ioctl_status;
	struct ffa_ioctl_msg_args direct_msg;
	memset(&direct_msg, 0, sizeof(direct_msg));

	direct_msg.dst_id = s->dest_partition_id;
	direct_msg.args[FFA_CALL_ARGS_IFACE_ID_OPCODE] =
		FFA_CALL_ARGS_COMBINE_IFACE_ID_OPCODE(FFA_CALL_MGMT_IFACE_ID, FFA_CALL_OPCODE_UNSHARE_BUF);
	direct_msg.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_LSW] = (uint32_t)s->shared_mem_handle;
	direct_msg.args[FFA_CALL_ARGS_SHARE_MEM_HANDLE_MSW] = (uint32_t)(s->shared_mem_handle >> 32);

	ioctl_status = ioctl(s->fd, FFA_IOC_MSG_SEND, &direct_msg);

	return ioctl_status;
}

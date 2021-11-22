/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mm_communicate_caller.h"
#include "carveout.h"
#include <arm_ffa_user.h>
#include <components/rpc/mm_communicate/common/mm_communicate_call_args.h>
#include <protocols/rpc/common/packed-c/status.h>
#include <protocols/common/mm/mm_smc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define KERNEL_MOD_REQ_VER_MAJOR 2
#define KERNEL_MOD_REQ_VER_MINOR 0
#define KERNEL_MOD_REQ_VER_PATCH 0

static rpc_call_handle call_begin(
	void *context,
	uint8_t **req_buf,
	size_t req_len);

static rpc_status_t call_invoke(
	void *context,
	rpc_call_handle handle,
	uint32_t opcode,
	rpc_opstatus_t *opstatus,
	uint8_t **resp_buf,
	size_t *resp_len);

static void call_end(
	void *context,
	rpc_call_handle handle);

static rpc_status_t mm_return_code_to_rpc_status(
	int32_t return_code);

bool mm_communicate_caller_check_version(void)
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

struct rpc_caller *mm_communicate_caller_init(
	struct mm_communicate_caller *s,
	const char *ffa_device_path)
{
	struct rpc_caller *base = &s->rpc_caller;

	rpc_caller_init(base, s);
	base->call_begin = call_begin;
	base->call_invoke = call_invoke;
	base->call_end = call_end;

	s->ffa_fd = -1;
	s->ffa_device_path = ffa_device_path;
	s->dest_partition_id = 0;
	s->comm_buffer = NULL;
	s->comm_buffer_size = 0;
	s->scrub_len = 0;
	s->req_len = 0;
	s->is_call_transaction_in_progess = false;
	s->serializer = NULL;

	return base;
}

void mm_communicate_caller_deinit(
	struct mm_communicate_caller *s)
{
	s->rpc_caller.context = NULL;
	s->rpc_caller.call_begin = NULL;
	s->rpc_caller.call_invoke = NULL;
	s->rpc_caller.call_end = NULL;

	call_end(s, s);
	mm_communicate_caller_close(s);
}

size_t mm_communicate_caller_discover(
	const struct mm_communicate_caller *s,
	const struct uuid_canonical *uuid,
	uint16_t *partition_ids,
	size_t discover_limit)
{
	size_t discover_count = 0;

	if (uuid && partition_ids && s->ffa_device_path) {
		int fd;

		fd = open(s->ffa_device_path, O_RDWR);

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

int mm_communicate_caller_open(
	struct mm_communicate_caller *s,
	uint16_t dest_partition_id,
	const EFI_GUID *svc_guid)
{
	int status = -1;

	s->serializer = mm_communicate_serializer_find(svc_guid);

	if (s->serializer && s->ffa_device_path) {

		s->ffa_fd = open(s->ffa_device_path, O_RDWR);

		if ((s->ffa_fd >= 0) && !s->comm_buffer) {

			status = carveout_claim(
				&s->comm_buffer,
				&s->comm_buffer_size);

			if (status == 0) {

				s->dest_partition_id = dest_partition_id;
			}
			else {
				/* Failed to claim carveout */
				s->comm_buffer = NULL;
				s->comm_buffer_size = 0;

				mm_communicate_caller_close(s);
			}
		}
	}

	if (status != 0) {

		s->serializer = NULL;
	}

	return status;
}

int mm_communicate_caller_close(
	struct mm_communicate_caller *s)
{
	if (s->ffa_fd >= 0) {

		close(s->ffa_fd);
		s->ffa_fd = -1;
		s->dest_partition_id = 0;
	}

	if (s->comm_buffer) {

		carveout_relinquish(s->comm_buffer, s->comm_buffer_size);
		s->comm_buffer = NULL;
		s->comm_buffer_size = 0;
	}

	s->serializer = NULL;

	s->is_call_transaction_in_progess = false;

	return ((s->ffa_fd < 0) && !s->comm_buffer) ? 0 : -1;
}

static rpc_call_handle call_begin(
	void *context,
	uint8_t **req_buf,
	size_t req_len)
{
	rpc_call_handle handle = NULL;
	struct mm_communicate_caller *s = (struct mm_communicate_caller*)context;
	size_t hdr_size = mm_communicate_serializer_header_size(s->serializer);
	*req_buf = NULL;

	if (!s->is_call_transaction_in_progess && hdr_size) {

		if (req_len + hdr_size <= s->comm_buffer_size) {

			s->is_call_transaction_in_progess = true;
			handle = s;

			s->req_len = req_len;
			*req_buf = &s->comm_buffer[hdr_size];

			s->scrub_len = hdr_size + req_len;
		}
		else {

			s->req_len = 0;
		}
	}

	return handle;
}

static rpc_status_t call_invoke(
	void *context,
	rpc_call_handle handle,
	uint32_t opcode,
	rpc_opstatus_t *opstatus,
	uint8_t **resp_buf,
	size_t *resp_len)
{
	struct mm_communicate_caller *s = (struct mm_communicate_caller*)context;

	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;
	*resp_len = 0;

	if ((handle == s) && s->is_call_transaction_in_progess) {

		mm_communicate_serializer_header_encode(s->serializer,
			s->comm_buffer, opcode, s->req_len);

		/* Make direct call to send the request */
		struct ffa_ioctl_msg_args direct_msg;
		memset(&direct_msg, 0, sizeof(direct_msg));

		direct_msg.dst_id = s->dest_partition_id;

		direct_msg.args[MM_COMMUNICATE_CALL_ARGS_COMM_BUFFER_ADDRESS] = (uintptr_t)s->comm_buffer;
		direct_msg.args[MM_COMMUNICATE_CALL_ARGS_COMM_BUFFER_SIZE] = s->comm_buffer_size;

		int kernel_op_status = ioctl(s->ffa_fd, FFA_IOC_MSG_SEND, &direct_msg);

		if (kernel_op_status == 0) {

			/* Kernel send operation completed normally */
			uint32_t mm_return_id = direct_msg.args[MM_COMMUNICATE_CALL_ARGS_RETURN_ID];
			int32_t mm_return_code = direct_msg.args[MM_COMMUNICATE_CALL_ARGS_RETURN_CODE];

			if (mm_return_id == ARM_SVC_ID_SP_EVENT_COMPLETE) {

				if (mm_return_code == MM_RETURN_CODE_SUCCESS) {

					mm_communicate_serializer_header_decode(s->serializer,
						s->comm_buffer, opstatus, resp_buf, resp_len);

					if (*resp_len > s->req_len) {

						s->scrub_len =
							mm_communicate_serializer_header_size(s->serializer) +
							*resp_len;
					}

					rpc_status = TS_RPC_CALL_ACCEPTED;
				}
				else {

					rpc_status = mm_return_code_to_rpc_status(mm_return_code);
				}
			}
		}
	}

	return rpc_status;
}

static void call_end(void *context, rpc_call_handle handle)
{
	struct mm_communicate_caller *s = (struct mm_communicate_caller*)context;

	if ((handle == s) && s->is_call_transaction_in_progess) {

		/* Call transaction complete */
		s->req_len = 0;
		s->is_call_transaction_in_progess = false;

		/* Scrub the comms buffer */
		memset(s->comm_buffer, 0, s->scrub_len);
		s->scrub_len = 0;
	}
}

static rpc_status_t mm_return_code_to_rpc_status(int32_t return_code)
{
	rpc_status_t rpc_status = TS_RPC_ERROR_INTERNAL;

	switch (return_code)
	{
		case MM_RETURN_CODE_NOT_SUPPORTED:
			rpc_status = TS_RPC_ERROR_INTERFACE_DOES_NOT_EXIST;
			break;
		case MM_RETURN_CODE_INVALID_PARAMETER:
			rpc_status = TS_RPC_ERROR_INVALID_PARAMETER;
			break;
		case MM_RETURN_CODE_DENIED:
			rpc_status = TS_RPC_ERROR_ACCESS_DENIED;
			break;
		case MM_RETURN_CODE_NO_MEMORY:
			rpc_status = TS_RPC_ERROR_RESOURCE_FAILURE;
			break;
		default:
			break;
	}

	return rpc_status;
}

/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_ffa_tee.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/tee.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_TEE_DEV_NUM 10

int ffa_tee_find_dev(char *path, size_t path_len)
{
	struct tee_ioctl_version_data vers = {0};
	char tmp_path[16] = {0};
	int fd = -1, rc = -1, print_len = -1;
	size_t n = 0;

	if (!path || !path_len)
		return -1;

	for (n = 0; n < MAX_TEE_DEV_NUM; n++) {
		/* Returns number of characters, excluding the null terminator */
		print_len = snprintf(tmp_path, sizeof(tmp_path), "/dev/tee%zu", n);

		fd = open(tmp_path, O_RDWR);
		if (fd < 0)
			continue;

		memset(&vers, 0, sizeof(vers));
		rc = ioctl(fd, TEE_IOC_VERSION, &vers);

		close(fd);

		/* Suitable device path was found */
		if (!rc && vers.impl_id == TEE_IMPL_ID_FFATEE) {
			/* Buffer is too short */
			if (path_len < print_len + 1)
				return -1;

			memcpy(path, tmp_path, print_len + 1);
			return 0;
		}
	}

	return -1;
}

int ffa_tee_open_session(int fd, uint16_t part_id)
{
	int rc = -1;
	struct tee_ioctl_open_session_arg *arg = NULL;
	struct tee_ioctl_param *params = NULL;
	const size_t arg_size = sizeof(struct tee_ioctl_open_session_arg) +
				1 * sizeof(struct tee_ioctl_param);
	union {
		struct tee_ioctl_open_session_arg arg;
		uint8_t data[arg_size];
	} buf;
	struct tee_ioctl_buf_data buf_data = {0};

	memset(&buf, 0, sizeof(buf));

	if (fd < 0) {
		printf("%s():%d error\n", __func__, __LINE__);
		return -1;
	}

	buf_data.buf_ptr = (uintptr_t)&buf;
	buf_data.buf_len = sizeof(buf);

	arg = &buf.arg;
	arg->num_params = 1;
	params = (struct tee_ioctl_param *)(arg + 1);

	params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	params[0].a = part_id;

	rc = ioctl(fd, TEE_IOC_OPEN_SESSION, &buf_data);
	if (rc) {
		printf("%s():%d error: %d\n", __func__, __LINE__, errno);
		return -1;
	}

	return 0;
}

void ffa_tee_close_session(int fd)
{
	struct tee_ioctl_close_session_arg arg = {0};

	if (ioctl(fd, TEE_IOC_CLOSE_SESSION, &arg))
		printf("%s():%d error: %d\n", __func__, __LINE__, errno);
}

static int ffa_tee_invoke_func(int fd, struct tee_ioctl_buf_data *buf_data)
{
	int rc = -1;

	rc = ioctl(fd, TEE_IOC_INVOKE, buf_data);
	if (rc) {
		printf("%s():%d error: %d\n", __func__, __LINE__, errno);
		return -1;
	}

	return 0;
}

int ffa_tee_send_msg(int fd, uint16_t iface_id, uint16_t opcode, size_t req_len, uint16_t encoding,
		     size_t *resp_len, int32_t *rpc_status, int32_t *opstatus)
{
	struct tee_ioctl_invoke_arg *arg = NULL;
	struct tee_ioctl_param *params = NULL;
	const size_t arg_size = sizeof(struct tee_ioctl_invoke_arg) +
				1 * sizeof(struct tee_ioctl_param);
	union {
		struct tee_ioctl_invoke_arg arg;
		uint8_t data[arg_size];
	} buf;
	struct tee_ioctl_buf_data buf_data = {0};
	int rc = -1;

	memset(&buf, 0, sizeof(buf));

	buf_data.buf_ptr = (uintptr_t)&buf;
	buf_data.buf_len = sizeof(buf);

	arg = &buf.arg;
	arg->func = CMD_SEND_MSG;
	arg->num_params = 1;
	params = (struct tee_ioctl_param *)(arg + 1);

	params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INOUT;
	params[0].a = CMD_SEND_MSG_PARAM_A(iface_id, opcode);
	params[0].b = CMD_SEND_MSG_PARAM_B(req_len);
	params[0].c = CMD_SEND_MSG_PARAM_C(encoding);

	rc = ffa_tee_invoke_func(fd, &buf_data);
	if (rc)
		return rc;

	*resp_len = CMD_SEND_MSG_RESP_LEN(params[0].a);
	*rpc_status = CMD_SEND_MSG_RPC_STATUS(params[0].b);
	*opstatus = CMD_SEND_MSG_OP_STATUS(params[0].b);

	return 0;
}

int ffa_tee_list_part_ids(int fd, const uint8_t *uuid, uint16_t *id_buf, size_t id_buf_cnt,
			  size_t *populated, bool *buf_short)
{
	struct tee_ioctl_invoke_arg *arg = NULL;
	struct tee_ioctl_param *params = NULL;
	const size_t arg_size = sizeof(struct tee_ioctl_invoke_arg) +
				2 * sizeof(struct tee_ioctl_param);
	union {
		struct tee_ioctl_invoke_arg arg;
		uint8_t data[arg_size];
	} buf;
	struct tee_ioctl_buf_data buf_data = {0};
	int rc = -1;

	memset(&buf, 0, sizeof(buf));

	buf_data.buf_ptr = (uintptr_t)&buf;
	buf_data.buf_len = sizeof(buf);

	arg = &buf.arg;
	arg->func = CMD_GET_PARTITION_LIST;
	arg->num_params = 2;
	params = (struct tee_ioctl_param *)(arg + 1);

	params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	memcpy(&params[0].a, uuid, 8);
	memcpy(&params[0].b, uuid + 8, 8);
	params[0].c = 0;

	params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INOUT;
	params[1].a = (uintptr_t)id_buf;
	params[1].b = id_buf_cnt;
	params[1].c = 0;

	rc = ffa_tee_invoke_func(fd, &buf_data);
	if (rc)
		return rc;

	*populated = params[1].a;
	*buf_short = (params[1].b != 0);

	return 0;
}

int ffa_tee_share_mem(int fd, size_t req_size, void **addr, size_t *size, int *id)
{
	struct tee_ioctl_shm_alloc_data data = {.size = req_size};
	int shm_fd = -1;

	shm_fd = ioctl(fd, TEE_IOC_SHM_ALLOC, &data);
	if (shm_fd < 0) {
		printf("%s():%d error: %d\n", __func__, __LINE__, errno);
		return -1;
	}

	*addr = mmap(NULL, data.size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (*addr == (void *)MAP_FAILED) {
		printf("%s():%d error: %d\n", __func__, __LINE__, errno);
		close(shm_fd);
		return -1;
	}
	close(shm_fd);
	*size = data.size;
	*id = data.id;

	return 0;
}

void ffa_tee_reclaim_mem(int fd, void *addr, size_t size)
{
	int rc = 0;

	(void)fd;

	rc = munmap(addr, size);
	if (rc)
		printf("%s():%d error: %d\n", __func__, __LINE__, errno);
}

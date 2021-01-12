/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 */

#ifndef LIBSP_TEST_MOCK_FFA_API_H_
#define LIBSP_TEST_MOCK_FFA_API_H_

#include <stdint.h>
#include "../include/ffa_api.h"

void expect_ffa_version(const uint32_t *version, ffa_result result);

void expect_ffa_features(
	uint32_t ffa_function_id,
	const struct ffa_interface_properties *interface_properties,
	ffa_result result);

void expect_ffa_rx_release(ffa_result result);

void expect_ffa_rxtx_map(const void *tx_buffer, const void *rx_buffer,
			 uint32_t page_count, ffa_result result);

void expect_ffa_rxtx_unmap(uint16_t id, ffa_result result);

void expect_ffa_partition_info_get(const struct ffa_uuid *uuid,
				   const uint32_t *count, ffa_result result);

void expect_ffa_id_get(const uint16_t *id, ffa_result result);

void expect_ffa_msg_wait(const struct ffa_direct_msg *msg, ffa_result result);

void expect_ffa_msg_send_direct_req(uint16_t source, uint16_t dest, uint32_t a0,
				    uint32_t a1, uint32_t a2, uint32_t a3,
				    uint32_t a4,
				    const struct ffa_direct_msg *msg,
				    ffa_result result);

void expect_ffa_msg_send_direct_resp(uint16_t source, uint16_t dest,
				     uint32_t a0, uint32_t a1, uint32_t a2,
				     uint32_t a3, uint32_t a4,
				     const struct ffa_direct_msg *msg,
				     ffa_result result);

void expect_ffa_mem_donate(uint32_t total_length, uint32_t fragment_length,
			   void *buffer_address, uint32_t page_count,
			   const uint64_t *handle, ffa_result result);

void expect_ffa_mem_donate_rxtx(uint32_t total_length, uint32_t fragment_length,
				const uint64_t *handle, ffa_result result);

void expect_ffa_mem_lend(uint32_t total_length, uint32_t fragment_length,
			 void *buffer_address, uint32_t page_count,
			 const uint64_t *handle, ffa_result result);

void expect_ffa_mem_lend_rxtx(uint32_t total_length, uint32_t fragment_length,
			      const uint64_t *handle, ffa_result result);

void expect_ffa_mem_share(uint32_t total_length, uint32_t fragment_length,
			  void *buffer_address, uint32_t page_count,
			  const uint64_t *handle, ffa_result result);

void expect_ffa_mem_share_rxtx(uint32_t total_length, uint32_t fragment_length,
			       const uint64_t *handle, ffa_result result);

void expect_ffa_mem_retrieve_req(uint32_t total_length,
				 uint32_t fragment_length, void *buffer_address,
				 uint32_t page_count,
				 const uint32_t *resp_total_length,
				 const uint32_t *resp_fragment_length,
				 ffa_result result);

void expect_ffa_mem_retrieve_req_rxtx(uint32_t total_length,
				      uint32_t fragment_length,
				      const uint32_t *resp_total_length,
				      const uint32_t *resp_fragment_length,
				      ffa_result result);

void expect_ffa_mem_relinquish(ffa_result result);

void expect_ffa_mem_reclaim(uint64_t handle, uint32_t flags, ffa_result result);

#endif /* FFA_LIBSP_TEST_MOCK_FFA_API_H_ */

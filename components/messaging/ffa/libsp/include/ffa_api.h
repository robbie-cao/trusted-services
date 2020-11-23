/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 */

#ifndef LIBSP_INCLUDE_FFA_API_H_
#define LIBSP_INCLUDE_FFA_API_H_

/**
 * @file  ffa_api.h
 * @brief The file contains wrapper functions around the FF-A interfaces
 *        described in sections 7-11 of the specification.
 */

#include "ffa_api_types.h"
#include "ffa_api_defines.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Setup and discovery interfaces
 */

/**
 * @brief      Queries the version of the Firmware Framework implementation at
 *             the FF-A instance.
 *
 * @param[out] version  Version number of the FF-A implementation
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_version(uint32_t *version);

/**
 * @brief      Queries whether the FF-A interface is implemented of the
 *             component at the higher EL and if it implements any optional
 *             features. The meaning of the interface_properties structure
 *             depends on the queried FF-A function and it is described in
 *             section 8.2 of the FF-A standard (v1.0).
 *
 * @param[in]  ffa_function_id       The function id of the queried FF-A
 *                                   function
 * @param[out] interface_properties  Used to encode any optional features
 *                                   implemented or any implementation details
 *                                   exported by the queried interface
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_features(uint32_t ffa_function_id,
			struct ffa_interface_properties *interface_properties);

/**
 * @brief      Relinquishes the ownership of the RX buffer after reading a
 *             message from it.
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_rx_release(void);

/**
 * @brief      Maps the RX/TX buffer pair in the callee's translation regime.
 *
 * @param[in]  tx_buffer   Base address of the TX buffer
 * @param[in]  rx_buffer   Base address of the RX buffer
 * @param[in]  page_count  Number of contiguous 4K pages allocated for each
 *                         buffer
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_rxtx_map(const void *tx_buffer, const void *rx_buffer,
			uint32_t page_count);

/**
 * @brief      Unmaps the RX/TX buffer pair in the callee's translation regime.
 *
 * @param[in]  id     ID of FF-A component that allocated the RX/TX buffer
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_rxtx_unmap(uint16_t id);

/**
 * @brief      Requests the SPM to return information about the partition of
 *             the system. Nil UUID can be used to return information about all
 *             the SPs of the system. The information is returned in the RX
 *             buffer of the caller as an array of ffa_partition_information
 *             structures.
 *
 * @param[in]  uuid   The uuid
 * @param[out] count  Count of partition information descriptors populated in
 *                    RX buffer of caller
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_partition_info_get(const struct ffa_uuid *uuid, uint32_t *count);

/**
 * @brief      Returns the 16 bit ID of the calling FF-A component
 *
 * @param      id    ID of the caller
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_id_get(uint16_t *id);

/**
 * CPU cycle management interfaces
 */

/**
 * @brief      Blocks the caller until a message is available or until an
 *             interrupt happens. It is also used to indicate the completion of
 *             the boot phase and the end of the interrupt handling.
 * @note       The ffa_interrupt_handler function can be called during the
 *             execution of this function.
 *
 * @param[out] msg   The incoming message
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_msg_wait(struct ffa_direct_msg *msg);

/** Messaging interfaces */

/**
 * @brief      Sends a partition message in parameter registers as a request and
 *             blocks until the response is available.
 * @note       The ffa_interrupt_handler function can be called during the
 *             execution of this function
 *
 * @param[in]  source            Source endpoint ID
 * @param[in]  dest              Destination endpoint ID
 * @param[in]  a0,a1,a2,a3,a4    Implementation defined message values
 * @param[out] msg               The response message
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_msg_send_direct_req(uint16_t source, uint16_t dest, uint32_t a0,
				   uint32_t a1, uint32_t a2, uint32_t a3,
				   uint32_t a4, struct ffa_direct_msg *msg);

/**
 * @brief      Sends a partition message in parameter registers as a response
 *             and blocks until the response is available.
 * @note       The ffa_interrupt_handler function can be called during the
 *             execution of this function
 *
 * @param[in]  source            Source endpoint ID
 * @param[in]  dest              Destination endpoint ID
 * @param[in]  a0,a1,a2,a3,a4    Implementation defined message values
 * @param[out] msg               The response message
 *
 * @return     The FF-A error status code
 */
ffa_result ffa_msg_send_direct_resp(uint16_t source, uint16_t dest, uint32_t a0,
				    uint32_t a1, uint32_t a2, uint32_t a3,
				    uint32_t a4, struct ffa_direct_msg *msg);

/**
 * @brief      Interrupt handler prototype. Must be implemented by another
 *             component.
 *
 * @param[in]  interrupt_id  The interrupt identifier
 */
void ffa_interrupt_handler(uint32_t interrupt_id);

#ifdef __cplusplus
}
#endif

#endif /* LIBSP_INCLUDE_FFA_API_H_ */

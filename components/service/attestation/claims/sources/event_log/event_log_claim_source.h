/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EVENT_LOG_CLAIM_SOURCE_H
#define EVENT_LOG_CLAIM_SOURCE_H

#include <stdint.h>
#include <stddef.h>
#include <service/attestation/claims/claim.h>
#include <service/attestation/claims/claim_source.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A claim source for accessing a TCG event log, stored as
 * a contiguous array of bytes.
 */
struct event_log_claim_source
{
    struct claim_source base;

    const uint8_t *event_log;
    size_t event_log_len;
};

/**
 * \brief Initializes a event_log_claim_source.
 *
 * \param[in] instance      The event_log_claim_source instance to initialze
 * \param[in] event_log     Pointer to the event log.
 * \param[in] event_log_len Byte length of the event logf
 *
 * \return The initialize base claim_source structure
 */
struct claim_source *event_log_claim_source_init(struct event_log_claim_source *instance,
                                const uint8_t *event_log, size_t event_log_len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EVENT_LOG_CLAIM_SOURCE_H */

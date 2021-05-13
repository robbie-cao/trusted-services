/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef REPORT_DUMP_H
#define REPORT_DUMP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dump a serialized report as an array of hex bytes.  Bytes are
 * output to stdout. This is useful for viewing the report contents
 * using an external CBOR decoder.
 */
void report_dump(const uint8_t *report, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REPORT_DUMP_H */

/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMMON_CRC32_H
#define COMMON_CRC32_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Calculate a CRC32 over the provided data
 *
 * \param[in]  	crc		    The starting CRC for previous data
 * \param[in]  	buf  		The buffer to calculate the CRC over
 * \param[in]  	size		Number of bytes in the buffer
 *
 * \return	The calculated CRC32
 */
uint32_t crc32(uint32_t crc, const unsigned char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_CRC32_H */

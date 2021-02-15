/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 * \file sfs_flash_ram.h
 *
 * \brief Implementations of the flash interface functions for an emulated flash
 *        device using RAM. See sfs_flash.h for full documentation of functions.
 */

#include "sfs_flash.h"

/**
 * \brief Initialize the Flash Interface.
 */
psa_status_t sfs_flash_ram_init(const struct sfs_flash_info_t *info);

/**
 * \brief Reads block data from the position specified by block ID and offset.
 */
psa_status_t sfs_flash_ram_read(const struct sfs_flash_info_t *info,
                                uint32_t block_id, uint8_t *buff, size_t offset,
                                size_t size);

/**
 * \brief Writes block data to the position specified by block ID and offset.
 */
psa_status_t sfs_flash_ram_write(const struct sfs_flash_info_t *info,
                                 uint32_t block_id, const uint8_t *buff,
                                 size_t offset, size_t size);

/**
 * \brief Flushes modifications to a block to flash.
 */
psa_status_t sfs_flash_ram_flush(const struct sfs_flash_info_t *info);

/**
 * \brief Erases block ID data.
 */
psa_status_t sfs_flash_ram_erase(const struct sfs_flash_info_t *info,
                                 uint32_t block_id);

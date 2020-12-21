/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_CRYPTO_OPEN_KEY_H
#define TS_CRYPTO_OPEN_KEY_H

#include <stdint.h>

/* Mandatory fixed sized input parameters */
struct __attribute__ ((__packed__)) ts_crypto_open_key_in
{
  uint32_t id;
};

/* Mandatory fixed sized output parameters */
struct __attribute__ ((__packed__)) ts_crypto_open_key_out
{
  uint32_t handle;
};

#endif /* TS_CRYPTO_OPEN_KEY_H */
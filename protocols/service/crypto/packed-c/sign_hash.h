/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_CRYPTO_SIGN_HASH_H
#define TS_CRYPTO_SIGN_HASH_H

#include <stdint.h>

/* Mandatory fixed sized input parameters */
struct __attribute__ ((__packed__)) ts_crypto_sign_hash_in
{
  uint32_t id;
  uint32_t alg;
};

/* Variable length input parameter tags */
enum
{
    TS_CRYPTO_SIGN_HASH_IN_TAG_HASH  = 1,
};

/* Variable length output parameter tags */
enum
{
    TS_CRYPTO_SIGN_HASH_OUT_TAG_SIGNATURE  = 1
};

#endif /* TS_CRYPTO_SIGN_HASH_H */
/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_CRYPTO_KEY_ATTRIBUTES_H
#define TS_CRYPTO_KEY_ATTRIBUTES_H

#include <stdint.h>

/* Key types */
#define TS_CRYPTO_KEY_TYPE_NONE                   (0x0000)
#define TS_CRYPTO_KEY_TYPE_RAW_DATA               (0x1001)
#define TS_CRYPTO_KEY_TYPE_HMAC                   (0x1100)
#define TS_CRYPTO_KEY_TYPE_DERIVE                 (0x1200)
#define TS_CRYPTO_KEY_TYPE_AES                    (0x2400)
#define TS_CRYPTO_KEY_TYPE_DES                    (0x2301)
#define TS_CRYPTO_KEY_TYPE_CAMELLIA               (0x2403)
#define TS_CRYPTO_KEY_TYPE_ARC4                   (0x2002)
#define TS_CRYPTO_KEY_TYPE_CHACHA20               (0x2004)
#define TS_CRYPTO_KEY_TYPE_RSA_PUBLIC_KEY         (0x4001)
#define TS_CRYPTO_KEY_TYPE_RSA_KEY_PAIR           (0x7001)
#define TS_CRYPTO_KEY_TYPE_ECC_PUBLIC_KEY_BASE    (0x4100)
#define TS_CRYPTO_KEY_TYPE_ECC_KEY_PAIR_BASE      (0x7100)
#define TS_CRYPTO_KEY_TYPE_ECC_CURVE_MASK         (0x00ff)
#define TS_CRYPTO_KEY_TYPE_DH_PUBLIC_KEY_BASE     (0x4200)
#define TS_CRYPTO_KEY_TYPE_DH_KEY_PAIR_BASE       (0x7200)
#define TS_CRYPTO_KEY_TYPE_DH_GROUP_MASK          (0x00ff)

/* ECC curves for use with ECC Key types */
#define TS_CRYPTO_ECC_CURVE_NONE                  (0x00)
#define TS_CRYPTO_ECC_CURVE_SECP_K1               (0x17)
#define TS_CRYPTO_ECC_CURVE_SECP_R1               (0x12)
#define TS_CRYPTO_ECC_CURVE_SECP_R2               (0x1b)
#define TS_CRYPTO_ECC_CURVE_SECT_K1               (0x27)
#define TS_CRYPTO_ECC_CURVE_SECT_R1               (0x22)
#define TS_CRYPTO_ECC_CURVE_SECT_R2               (0x2b)
#define TS_CRYPTO_ECC_CURVE_BRAINPOOL_P_R1        (0x30)
#define TS_CRYPTO_ECC_CURVE_MONTGOMERY            (0x41)

/* Diffie-Hellman groups for use with DH key types */
#define TS_CRYPTO_DH_GROUP_NONE                   (0x00)
#define TS_CRYPTO_DH_GROUP_RFC7919                (0x03)

/* Crypto algorithms */
#define TS_CRYPTO_ALG_NONE                        (0x00000000)
#define TS_CRYPTO_ALG_HASH_MASK                   (0x000000ff)
#define TS_CRYPTO_ALG_MD2                         (0x01000001)
#define TS_CRYPTO_ALG_MD4                         (0x01000002)
#define TS_CRYPTO_ALG_MD5                         (0x01000003)
#define TS_CRYPTO_ALG_RIPEMD160                   (0x01000004)
#define TS_CRYPTO_ALG_SHA_1                       (0x01000005)
#define TS_CRYPTO_ALG_SHA_224                     (0x01000008)
#define TS_CRYPTO_ALG_SHA_256                     (0x01000009)
#define TS_CRYPTO_ALG_SHA_384                     (0x0100000a)
#define TS_CRYPTO_ALG_SHA_512                     (0x0100000b)
#define TS_CRYPTO_ALG_SHA_512_224                 (0x0100000c)
#define TS_CRYPTO_ALG_SHA_512_256                 (0x0100000d)
#define TS_CRYPTO_ALG_SHA3_224                    (0x01000010)
#define TS_CRYPTO_ALG_SHA3_256                    (0x01000011)
#define TS_CRYPTO_ALG_SHA3_384                    (0x01000012)
#define TS_CRYPTO_ALG_SHA3_512                    (0x01000013)
#define TS_CRYPTO_ALG_CBC_MAC                     (0x02c00001)
#define TS_CRYPTO_ALG_CMAC                        (0x02c00002)
#define TS_CRYPTO_ALG_ARC4                        (0x04800001)
#define TS_CRYPTO_ALG_CHACHA20                    (0x04800005)
#define TS_CRYPTO_ALG_CTR                         (0x04c00001)
#define TS_CRYPTO_ALG_CFB                         (0x04c00002)
#define TS_CRYPTO_ALG_OFB                         (0x04c00003)
#define TS_CRYPTO_ALG_XTS                         (0x044000ff)
#define TS_CRYPTO_ALG_CBC_NO_PADDING              (0x04600100)
#define TS_CRYPTO_ALG_CBC_PKCS7                   (0x04600101)
#define TS_CRYPTO_ALG_AEAD_FROM_BLOCK_FLAG        (0x00400000)
#define TS_CRYPTO_ALG_CCM                         (0x06401001)
#define TS_CRYPTO_ALG_GCM                         (0x06401002)
#define TS_CRYPTO_ALG_CHACHA20_POLY1305           (0x06001005)
#define TS_CRYPTO_ALG_RSA_PKCS1V15_SIGN_BASE      (0x10020000)
#define TS_CRYPTO_ALG_RSA_PSS_BASE                (0x10030000)
#define TS_CRYPTO_ALG_ECDSA_BASE                  (0x10060000)
#define TS_CRYPTO_ALG_DETERMINISTIC_ECDSA_BASE    (0x10070000)
#define TS_CRYPTO_ALG_RSA_PKCS1V15_CRYPT          (0x12020000)
#define TS_CRYPTO_ALG_RSA_OAEP_BASE               (0x12030000)
#define TS_CRYPTO_ALG_HKDF_BASE                   (0x20000100)
#define TS_CRYPTO_ALG_TLS12_PRF_BASE              (0x20000200)
#define TS_CRYPTO_ALG_TLS12_PSK_TO_MS_BASE        (0x20000300)
#define TS_CRYPTO_ALG_KEY_DERIVATION_MASK         (0x0803ffff)
#define TS_CRYPTO_ALG_KEY_AGREEMENT_MASK          (0x10fc0000)
#define TS_CRYPTO_ALG_FFDH                        (0x30100000)
#define TS_CRYPTO_ALG_ECDH                        (0x30200000)

/* Key lifetime */
#define TS_CRYPTO_KEY_LIFETIME_VOLATILE           (0x00000000)
#define TS_CRYPTO_KEY_LIFETIME_PERSISTENT         (0x00000001)

/* Key usage constraints */
#define TS_CRYPTO_KEY_USAGE_NONE                  (0x00000000)
#define TS_CRYPTO_KEY_USAGE_EXPORT                (0x00000001)
#define TS_CRYPTO_KEY_USAGE_COPY                  (0x00000002)
#define TS_CRYPTO_KEY_USAGE_ENCRYPT               (0x00000100)
#define TS_CRYPTO_KEY_USAGE_DECRYPT               (0x00000200)
#define TS_CRYPTO_KEY_USAGE_SIGN_HASH             (0x00000400)
#define TS_CRYPTO_KEY_USAGE_VERIFY_HASH           (0x00000800)
#define TS_CRYPTO_KEY_USAGE_DERIVE                (0x00001000)

/* Key policy to define what key can be used for */
struct __attribute__ ((__packed__)) ts_crypto_key_policy
{
  uint32_t usage;
  uint32_t alg;
};

/* Key attributes object */
struct __attribute__ ((__packed__)) ts_crypto_key_attributes
{
  uint32_t type;
  uint32_t key_bits;
  uint32_t lifetime;
  uint32_t id;
  struct ts_crypto_key_policy policy;
};

#endif /* TS_CRYPTO_KEY_ATTRIBUTES_H */
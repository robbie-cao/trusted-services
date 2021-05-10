/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <psa/crypto.h>
#include <protocols/service/crypto/packed-c/key_attributes.h>
#include <CppUTest/TestHarness.h>

/*
 * Check alignment of Crypto service packed-c protocol definitions for
 * alignment with PSA C API definitions.
 */
TEST_GROUP(CryptoProtocolPackedcChecks)
{

};

TEST(CryptoProtocolPackedcChecks, checkKeyType)
{
    /*
     * Check alignment between PSA and protobuf key type definitions
     */
    CHECK_EQUAL(PSA_KEY_TYPE_RAW_DATA, TS_CRYPTO_KEY_TYPE_RAW_DATA);
    CHECK_EQUAL(PSA_KEY_TYPE_HMAC, TS_CRYPTO_KEY_TYPE_HMAC);
    CHECK_EQUAL(PSA_KEY_TYPE_DERIVE, TS_CRYPTO_KEY_TYPE_DERIVE);
    CHECK_EQUAL(PSA_KEY_TYPE_AES, TS_CRYPTO_KEY_TYPE_AES);
    CHECK_EQUAL(PSA_KEY_TYPE_DES, TS_CRYPTO_KEY_TYPE_DES);
    CHECK_EQUAL(PSA_KEY_TYPE_CAMELLIA, TS_CRYPTO_KEY_TYPE_CAMELLIA);
    CHECK_EQUAL(PSA_KEY_TYPE_ARC4, TS_CRYPTO_KEY_TYPE_ARC4);
    CHECK_EQUAL(PSA_KEY_TYPE_CHACHA20, TS_CRYPTO_KEY_TYPE_CHACHA20);
    CHECK_EQUAL(PSA_KEY_TYPE_RSA_PUBLIC_KEY, TS_CRYPTO_KEY_TYPE_RSA_PUBLIC_KEY);
    CHECK_EQUAL(PSA_KEY_TYPE_RSA_KEY_PAIR, TS_CRYPTO_KEY_TYPE_RSA_KEY_PAIR);
}

TEST(CryptoProtocolPackedcChecks, checkEccCurve)
{
    /*
     * ECC curves for use with ECC Key types
     */
    CHECK_EQUAL(PSA_ECC_FAMILY_SECP_K1, TS_CRYPTO_ECC_FAMILY_SECP_K1);
    CHECK_EQUAL(PSA_ECC_FAMILY_SECP_R1, TS_CRYPTO_ECC_FAMILY_SECP_R1);
    CHECK_EQUAL(PSA_ECC_FAMILY_SECP_R2, TS_CRYPTO_ECC_FAMILY_SECP_R2);
    CHECK_EQUAL(PSA_ECC_FAMILY_SECT_K1, TS_CRYPTO_ECC_FAMILY_SECT_K1);
    CHECK_EQUAL(PSA_ECC_FAMILY_SECT_R1, TS_CRYPTO_ECC_FAMILY_SECT_R1);
    CHECK_EQUAL(PSA_ECC_FAMILY_SECT_R2, TS_CRYPTO_ECC_FAMILY_SECT_R2);
    CHECK_EQUAL(PSA_ECC_FAMILY_BRAINPOOL_P_R1, TS_CRYPTO_ECC_FAMILY_BRAINPOOL_P_R1);
    CHECK_EQUAL(PSA_ECC_FAMILY_MONTGOMERY, TS_CRYPTO_ECC_FAMILY_MONTGOMERY);
}

TEST(CryptoProtocolPackedcChecks, checkDhGroup)
{
    /*
     * Diffie-Hellman groups for use with DH key types
     */
    CHECK_EQUAL(PSA_DH_FAMILY_RFC7919, TS_CRYPTO_DH_FAMILY_RFC7919);
}

TEST(CryptoProtocolPackedcChecks, checkAlg)
{
    /*
     * Crypto algorithms
     */
    CHECK_EQUAL(PSA_ALG_MD2, TS_CRYPTO_ALG_MD2);
    CHECK_EQUAL(PSA_ALG_MD4, TS_CRYPTO_ALG_MD4);
    CHECK_EQUAL(PSA_ALG_MD5, TS_CRYPTO_ALG_MD5);
    CHECK_EQUAL(PSA_ALG_RIPEMD160, TS_CRYPTO_ALG_RIPEMD160);
    CHECK_EQUAL(PSA_ALG_SHA_1, TS_CRYPTO_ALG_SHA_1);
    CHECK_EQUAL(PSA_ALG_SHA_224, TS_CRYPTO_ALG_SHA_224);
    CHECK_EQUAL(PSA_ALG_SHA_256, TS_CRYPTO_ALG_SHA_256);
    CHECK_EQUAL(PSA_ALG_SHA_384, TS_CRYPTO_ALG_SHA_384);
    CHECK_EQUAL(PSA_ALG_SHA_512, TS_CRYPTO_ALG_SHA_512);
    CHECK_EQUAL(PSA_ALG_SHA_512_224, TS_CRYPTO_ALG_SHA_512_224);
    CHECK_EQUAL(PSA_ALG_SHA_512_256, TS_CRYPTO_ALG_SHA_512_256);
    CHECK_EQUAL(PSA_ALG_SHA3_224, TS_CRYPTO_ALG_SHA3_224);
    CHECK_EQUAL(PSA_ALG_SHA3_256, TS_CRYPTO_ALG_SHA3_256);
    CHECK_EQUAL(PSA_ALG_SHA3_384, TS_CRYPTO_ALG_SHA3_384);
    CHECK_EQUAL(PSA_ALG_SHA3_512, TS_CRYPTO_ALG_SHA3_512);
    CHECK_EQUAL(PSA_ALG_CBC_MAC, TS_CRYPTO_ALG_CBC_MAC);
    CHECK_EQUAL(PSA_ALG_CMAC, TS_CRYPTO_ALG_CMAC);
    CHECK_EQUAL(PSA_ALG_STREAM_CIPHER, TS_CRYPTO_ALG_STREAM_CIPHER);
    CHECK_EQUAL(PSA_ALG_CTR, TS_CRYPTO_ALG_CTR);
    CHECK_EQUAL(PSA_ALG_CFB, TS_CRYPTO_ALG_CFB);
    CHECK_EQUAL(PSA_ALG_OFB, TS_CRYPTO_ALG_OFB);
    CHECK_EQUAL(PSA_ALG_XTS, TS_CRYPTO_ALG_XTS);
    CHECK_EQUAL(PSA_ALG_CBC_NO_PADDING, TS_CRYPTO_ALG_CBC_NO_PADDING);
    CHECK_EQUAL(PSA_ALG_CBC_PKCS7, TS_CRYPTO_ALG_CBC_PKCS7);
    CHECK_EQUAL(PSA_ALG_CCM, TS_CRYPTO_ALG_CCM);
    CHECK_EQUAL(PSA_ALG_GCM, TS_CRYPTO_ALG_GCM);
    CHECK_EQUAL(PSA_ALG_CHACHA20_POLY1305, TS_CRYPTO_ALG_CHACHA20_POLY1305);
    CHECK_EQUAL(PSA_ALG_RSA_PKCS1V15_CRYPT, TS_CRYPTO_ALG_RSA_PKCS1V15_CRYPT);
    CHECK_EQUAL(PSA_ALG_FFDH, TS_CRYPTO_ALG_FFDH);
    CHECK_EQUAL(PSA_ALG_ECDH, TS_CRYPTO_ALG_ECDH);
}

TEST(CryptoProtocolPackedcChecks, checkKeyLifetime)
{
    /*
     * Key lifetime
     */
    CHECK_EQUAL(PSA_KEY_LIFETIME_VOLATILE, TS_CRYPTO_KEY_LIFETIME_VOLATILE);
    CHECK_EQUAL(PSA_KEY_LIFETIME_PERSISTENT, TS_CRYPTO_KEY_LIFETIME_PERSISTENT);
}

TEST(CryptoProtocolPackedcChecks, checkKeyUsage)
{
    /*
     * Key usage constraints
     */
    CHECK_EQUAL(PSA_KEY_USAGE_EXPORT, TS_CRYPTO_KEY_USAGE_EXPORT);
    CHECK_EQUAL(PSA_KEY_USAGE_COPY, TS_CRYPTO_KEY_USAGE_COPY);
    CHECK_EQUAL(PSA_KEY_USAGE_ENCRYPT, TS_CRYPTO_KEY_USAGE_ENCRYPT);
    CHECK_EQUAL(PSA_KEY_USAGE_DECRYPT, TS_CRYPTO_KEY_USAGE_DECRYPT);
    CHECK_EQUAL(PSA_KEY_USAGE_SIGN_HASH, TS_CRYPTO_KEY_USAGE_SIGN_HASH);
    CHECK_EQUAL(PSA_KEY_USAGE_VERIFY_HASH, TS_CRYPTO_KEY_USAGE_VERIFY_HASH);
    CHECK_EQUAL(PSA_KEY_USAGE_DERIVE, TS_CRYPTO_KEY_USAGE_DERIVE);
}
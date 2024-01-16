/*
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_LIBMBEDX509_H
#define CONFIG_LIBMBEDX509_H

/*
 * MbedTLS configuration for building libmbedcrypto and libx509 to act as a backend
 * for the crypto service provider running in an isolated secure processing environment.
 */
#define MBEDTLS_PSA_CRYPTO_CONFIG
#define MBEDTLS_NO_UDBL_DIVISION
#undef MBEDTLS_HAVE_TIME
#undef MBEDTLS_HAVE_TIME_DATE
#undef MBEDTLS_FS_IO
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#undef MBEDTLS_SELF_TEST
#undef MBEDTLS_PLATFORM_C
#undef MBEDTLS_PSA_ITS_FILE_C
#undef MBEDTLS_TIMING_C
#undef MBEDTLS_AESNI_C
#undef MBEDTLS_AESCE_C
#undef MBEDTLS_PADLOCK_C

#define MBEDTLS_BIGNUM_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRL_PARSE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_PKCS7_C

#define MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER
#define BACKEND_CRYPTO_API_ADD_PREFIX(f) __mbedtls_backend_##f
#include "../../../components/service/crypto/backend/prefixed_crypto_api.h"

#endif /* CONFIG_LIBMBEDX509_H */

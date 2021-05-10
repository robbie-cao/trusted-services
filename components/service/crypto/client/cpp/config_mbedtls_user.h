/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_MBEDTLS_USER_H
#define CONFIG_MBEDTLS_USER_H

/* Configuration for crypto client component */

#define MBEDTLS_NO_UDBL_DIVISION
#undef MBEDTLS_HAVE_TIME
#undef MBEDTLS_HAVE_TIME_DATE
#define MBEDTLS_TEST_NULL_ENTROPY
#undef MBEDTLS_FS_IO
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_NO_PLATFORM_ENTROPY
#undef MBEDTLS_SELF_TEST
#undef MBEDTLS_AESNI_C
#define MBEDTLS_CMAC_C
#undef MBEDTLS_PADLOCK_C
#undef MBEDTLS_PLATFORM_C
#undef MBEDTLS_PSA_CRYPTO_STORAGE_C
#undef MBEDTLS_PSA_ITS_FILE_C
#undef MBEDTLS_TIMING_C

#endif /* CONFIG_MBEDTLS_USER_H */

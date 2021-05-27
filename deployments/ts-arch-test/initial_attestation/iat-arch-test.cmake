#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Define test suite to build.  Used by the psa_arch_tests external component
#  to configure what test suite gets built.
#-------------------------------------------------------------------------------
set(TS_ARCH_TEST_SUITE INITIAL_ATTESTATION CACHE STRING "Arch test suite")

#-------------------------------------------------------------------------------
#  Add attestation specific components.
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts-arch-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/service/attestation/include"
)

# Configuration for mbedcrypto
set(MBEDTLS_USER_CONFIG_FILE
	"${TS_ROOT}/components/service/crypto/client/cpp/config_mbedtls_user.h"
	CACHE STRING "Configuration file for mbedcrypto")

# Mbed TLS provides libmbedcrypto
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(ts-arch-test PRIVATE mbedcrypto)

# Export psa crypto API
list(APPEND PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS ${PSA_CRYPTO_API_INCLUDE})

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  ts-arch-test
#-------------------------------------------------------------------------------
include(../../ts-arch-test.cmake REQUIRED)

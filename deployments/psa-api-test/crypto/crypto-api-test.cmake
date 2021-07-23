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
set(TS_ARCH_TEST_SUITE CRYPTO CACHE STRING "Arch test suite")

#-------------------------------------------------------------------------------
#  Extend the arch test build configuration to include tests missing from the
#  default configuration.
#-------------------------------------------------------------------------------
set(TS_ARCH_TEST_EXTERNAL_DEFS
	-DCRYPTO_1_0
	CACHE STRING "Arch test external defines")

#-------------------------------------------------------------------------------
#  The arch test build system puts its build output under a test suite specific
#  subdirectory.  The subdirectory name is different from the test suite name
#  so an additional define is needed to obtain the built library.
#-------------------------------------------------------------------------------
set(TS_ARCH_TEST_BUILD_SUBDIR crypto CACHE STRING "Arch test build subdirectory")

#-------------------------------------------------------------------------------
#  Crypto specific components
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "psa-api-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/service/crypto/include"
		"components/service/crypto/client/psa"
)

target_sources(psa-api-test PRIVATE
	${TS_ROOT}/deployments/psa-api-test/crypto/crypto_locator.c
)

#-------------------------------------------------------------------------------
#  Advertise PSA API include paths to PSA Arch tests
#
#-------------------------------------------------------------------------------
list(APPEND PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS ${PSA_CRYPTO_API_INCLUDE})

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  psa-api-test
#-------------------------------------------------------------------------------
include(../../psa-api-test.cmake REQUIRED)

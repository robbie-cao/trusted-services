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

# Export psa crypto API
list(APPEND PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS ${PSA_CRYPTO_API_INCLUDE})

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  psa-api-test
#-------------------------------------------------------------------------------
include(../../psa-api-test.cmake REQUIRED)

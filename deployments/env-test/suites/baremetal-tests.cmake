#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Baremetal driver test cases to test paltform driver operation from within
# a secure execution environment.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Components-under-test and test cases baremetal platform tests.
#
#-------------------------------------------------------------------------------
add_components(TARGET "env-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/service/crypto/backend/mbedcrypto"
		"components/service/crypto/backend/mbedcrypto/trng_adapter/platform"
		"components/service/crypto/backend/mbedcrypto/trng_adapter/test"
		"components/service/secure_storage/include"
		"components/service/secure_storage/frontend/psa/its"
		"components/service/secure_storage/backend/secure_storage_client"
		"components/config/test/sp"
)

target_sources(env-test PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/registration/baremetal_tests.c
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Mbed TLS provides libmbedcrypto
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(env-test PRIVATE mbedcrypto)

#-------------------------------------------------------------------------------
#  This test suite depends on platform specific drivers
#
#-------------------------------------------------------------------------------
add_platform(TARGET "env-test")
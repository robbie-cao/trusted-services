#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'env-test' for
#  different environments.  Used for running tests that validate hardwarw
#  backed services available from within a secure execution environment.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Components that are common across all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "env-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
	"components/common/fdt"
	"components/common/tlv"
	"components/config/ramstore"
	"components/rpc/common/interface"
	"components/rpc/common/caller"
	"components/service/common/include"
	"components/service/common/client"
	"components/service/common/provider"
	"components/service/test_runner/provider"
	"components/service/test_runner/provider/serializer/packed-c"
	"components/service/test_runner/provider/backend/null"
	"components/service/test_runner/provider/backend/simple_c"
	"components/service/crypto/backend/mbedcrypto"
	"components/service/crypto/backend/mbedcrypto/trng_adapter/platform"
	"components/service/crypto/backend/mbedcrypto/trng_adapter/test"
	"components/service/secure_storage/include"
	"components/service/secure_storage/frontend/psa/its"
	"components/service/secure_storage/backend/secure_storage_client"
	"protocols/rpc/common/packed-c"
)

#-------------------------------------------------------------------------------
#  Deployment specific source files
#-------------------------------------------------------------------------------
target_sources(env-test PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/env_test.c
	${CMAKE_CURRENT_LIST_DIR}/common/env_test_tests.c
)

target_include_directories(env-test PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Get libc include dir
get_property(LIBC_INCLUDE_PATH TARGET c PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

# Mbed TLS provides libmbedcrypto
list(APPEND MBEDTLS_EXTRA_INCLUDES ${LIBC_INCLUDE_PATH})
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(env-test PRIVATE mbedcrypto)

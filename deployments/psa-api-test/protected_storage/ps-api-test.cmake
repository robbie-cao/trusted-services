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
set(TS_ARCH_TEST_SUITE PROTECTED_STORAGE CACHE STRING "Arch test suite")

#-------------------------------------------------------------------------------
#  The arch test build system puts its build output under a test suite specific
#  subdirectory.  The subdirectory name is different from the test suite name
#  so an additional define is needed to obtain the built library.
#-------------------------------------------------------------------------------
set(TS_ARCH_TEST_BUILD_SUBDIR storage CACHE STRING "Arch test build subdirectory")

#-------------------------------------------------------------------------------
#  Crypto specific components
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "${PROJECT_NAME}"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/service/secure_storage/include"
		"components/service/secure_storage/frontend/psa/ps"
		"components/service/secure_storage/backend/secure_storage_client"
)

target_sources(${PROJECT_NAME} PRIVATE
	${TS_ROOT}/deployments/psa-api-test/protected_storage/ps_locator.c
)

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  psa-api-test
#-------------------------------------------------------------------------------
include(../../psa-api-test.cmake REQUIRED)

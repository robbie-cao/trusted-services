#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'psa-api-test' for
#  different environments.  Used for running PSA API tests.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Use libts for locating and accessing services. An appropriate version of
#  libts will be imported for the enviroment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(psa-api-test PRIVATE libts)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "psa-api-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/tlv"
		"components/service/common/include"
		"components/rpc/common/caller"
		"components/rpc/common/logging"
)

target_sources(psa-api-test PRIVATE
	${TS_ROOT}/deployments/psa-api-test/arch_test_runner.c
)

#-------------------------------------------------------------------------------
#  Export project header paths for arch tests
#
#-------------------------------------------------------------------------------
get_target_property(_include_paths psa-api-test INCLUDE_DIRECTORIES)
list(APPEND PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS ${_include_paths})

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# psa-arch-tests
include(${TS_ROOT}/external/psa_arch_tests/psa_arch_tests.cmake)
target_link_libraries(psa-api-test PRIVATE val_nspe test_combine pal_nspe)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS psa-api-test RUNTIME DESTINATION ${TS_ENV}/bin)

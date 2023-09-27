#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
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
#  libts will be imported for the environment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE libts::ts)

#-------------------------------------------------------------------------------
#  Components that are common across all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "${PROJECT_NAME}"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/tlv"
		"components/service/common/client"
		"components/service/common/include"
)

target_sources(${PROJECT_NAME} PRIVATE
	${TS_ROOT}/deployments/psa-api-test/arch_test_runner.c
)

#-------------------------------------------------------------------------------
#  Export project header paths for arch tests
#
#-------------------------------------------------------------------------------
get_target_property(_include_paths ${PROJECT_NAME} INCLUDE_DIRECTORIES)
list(APPEND PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS ${_include_paths})

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# psa-arch-tests
include(${TS_ROOT}/external/psa_arch_tests/psa_arch_tests.cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE val_nspe test_combine pal_nspe)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${TS_ENV}/bin)

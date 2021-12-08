#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(PSA_ARCH_TESTS_URL "https://github.com/ARM-software/psa-arch-tests.git" CACHE STRING "psa-arch-tests repository URL")
set(PSA_ARCH_TESTS_REFSPEC "2a1852252a9b9af655cbe02d5d3c930952d0d798" CACHE STRING "psa-arch-tests v22.01_API1.4_ADAC_BETA")
set(PSA_ARCH_TESTS_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/psa_arch_tests-src" CACHE PATH "psa-arch-tests source.")

set(GIT_OPTIONS
	GIT_REPOSITORY ${PSA_ARCH_TESTS_URL}
	GIT_TAG ${PSA_ARCH_TESTS_REFSPEC}
	GIT_SHALLOW TRUE
	PATCH_COMMAND git stash
		COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/modify_attest_config.patch
)

# Ensure list of include paths is separated correctly
#string(REPLACE ";" "\\;" PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS "${PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS}")

# Ensure list of defines is separated correctly
string(REPLACE ";" " " PSA_ARCH_TEST_EXTERNAL_DEFS "${PSA_ARCH_TEST_EXTERNAL_DEFS}")

include(${TS_ROOT}/tools/cmake/common/LazyFetch.cmake REQUIRED)
LazyFetch_MakeAvailable(DEP_NAME psa_arch_tests
	FETCH_OPTIONS "${GIT_OPTIONS}"
	SOURCE_DIR ${PSA_ARCH_TESTS_SOURCE_DIR}
	SOURCE_SUBDIR "api-tests"
	CACHE_FILE "${CMAKE_CURRENT_LIST_DIR}/pas-arch-test-init-cache.cmake.in"
	)

# Create targets for generated libraries
add_library(test_combine STATIC IMPORTED)
set_property(TARGET test_combine PROPERTY IMPORTED_LOCATION
	"${psa_arch_tests_BINARY_DIR}/dev_apis/${TS_ARCH_TEST_BUILD_SUBDIR}/test_combine${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
	"${psa_arch_tests_BINARY_DIR}/dev_apis/${TS_ARCH_TEST_BUILD_SUBDIR}/test_combine${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
	"${PSA_ARCH_TESTS_SOURCE_DIR}/api-tests/val/nspe/val_entry.h")

add_library(val_nspe STATIC IMPORTED)
set_property(TARGET val_nspe PROPERTY IMPORTED_LOCATION
	"${psa_arch_tests_BINARY_DIR}/val/val_nspe${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(pal_nspe STATIC IMPORTED)
set_property(TARGET pal_nspe PROPERTY IMPORTED_LOCATION
	"${psa_arch_tests_BINARY_DIR}/platform/pal_nspe${CMAKE_STATIC_LIBRARY_SUFFIX}")

#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Determine the number of processes to run while running parallel builds.
# Pass -DPROCESSOR_COUNT=<n> to cmake to override.
if(NOT DEFINED PROCESSOR_COUNT)
	include(ProcessorCount)
	ProcessorCount(PROCESSOR_COUNT)
	set(PROCESSOR_COUNT ${PROCESSOR_COUNT} CACHE STRING "Number of cores to use for parallel builds.")
endif()

set(PSA_ARCH_TESTS_URL "https://github.com/ARM-software/psa-arch-tests.git" CACHE STRING "psa-arch-tests repository URL")
set(PSA_ARCH_TESTS_REFSPEC "bfc75bdbb7181e9482864803300ae56cc9dbb1b5" CACHE STRING "psa-arch-tests git refspec")
set(PSA_ARCH_TESTS_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/psa-arch-tests_install" CACHE PATH "psa-arch-tests installation directory")
set(PSA_ARCH_TESTS_PACKAGE_PATH "${PSA_ARCH_TESTS_INSTALL_PATH}/libpsa-arch-tests/cmake" CACHE PATH "psa-arch-tests CMake package directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching psa-arch-tests
FetchContent_Declare(
	psa-arch-tests
	GIT_REPOSITORY ${PSA_ARCH_TESTS_URL}
	GIT_TAG ${PSA_ARCH_TESTS_REFSPEC}
	GIT_SHALLOW TRUE
	PATCH_COMMAND git stash --include-untracked
		COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/add_inherit_toolchain.patch
)

# FetchContent_GetProperties exports psa-arch-tests_SOURCE_DIR and psa-arch-tests_BINARY_DIR variables
FetchContent_GetProperties(psa-arch-tests)
if(NOT psa-arch-tests_POPULATED)
	message(STATUS "Fetching psa-arch-tests")
	FetchContent_Populate(psa-arch-tests)
endif()

# Ensure list of include paths is separated correctly
string(REPLACE ";" "\\;" PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS "${PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS}")

# Configure the psa-arch-test library
execute_process(COMMAND
	${CMAKE_COMMAND}
			-DTOOLCHAIN=INHERIT
			-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
			-DPSA_INCLUDE_PATHS=${PSA_ARCH_TESTS_EXTERNAL_INCLUDE_PATHS}
			-DSUITE=${TS_ARCH_TEST_SUITE}
			-DCMAKE_VERBOSE_MAKEFILE=OFF
			-DTARGET=tgt_dev_apis_linux
			-GUnix\ Makefiles
			${psa-arch-tests_SOURCE_DIR}/api-tests
		WORKING_DIRECTORY
			${psa-arch-tests_BINARY_DIR}
		RESULT_VARIABLE _exec_error
)

# Build the library
if (_exec_error)
	message(FATAL_ERROR "Configuration step of psa-arch-tests runtime failed with ${_exec_error}.")
endif()

execute_process(COMMAND
		${CMAKE_COMMAND} --build ${psa-arch-tests_BINARY_DIR} --parallel ${PROCESSOR_COUNT}
		RESULT_VARIABLE _exec_error
	)
if (_exec_error)
	message(FATAL_ERROR "Build step of psa-arch-tests runtime failed with ${_exec_error}.")
endif()

# Create targets for generated libraries
add_library(test_combine STATIC IMPORTED)
set_property(TARGET test_combine PROPERTY IMPORTED_LOCATION "${psa-arch-tests_BINARY_DIR}/dev_apis/crypto/test_combine${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(val_nspe STATIC IMPORTED)
set_property(TARGET val_nspe PROPERTY IMPORTED_LOCATION "${psa-arch-tests_BINARY_DIR}/val/val_nspe${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(pal_nspe STATIC IMPORTED)
set_property(TARGET pal_nspe PROPERTY IMPORTED_LOCATION "${psa-arch-tests_BINARY_DIR}/platform/pal_nspe${CMAKE_STATIC_LIBRARY_SUFFIX}")

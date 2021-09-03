#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# t_cose is a library for signing CBOR tokens using COSE_Sign1
#-------------------------------------------------------------------------------

# Determine the number of processes to run while running parallel builds.
# Pass -DPROCESSOR_COUNT=<n> to cmake to override.
if(NOT DEFINED PROCESSOR_COUNT)
	include(ProcessorCount)
	ProcessorCount(PROCESSOR_COUNT)
	set(PROCESSOR_COUNT ${PROCESSOR_COUNT} CACHE STRING "Number of cores to use for parallel builds.")
endif()

# External component details
set(T_COSE_URL "https://github.com/laurencelundblade/t_cose.git" CACHE STRING "t_cose repository URL")
set(T_COSE_REFSPEC "master" CACHE STRING "t_cose git refspec")
set(T_COSE_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/t_cose_install" CACHE PATH "t_cose installation directory")
set(T_COSE_PACKAGE_PATH "${T_COSE_INSTALL_PATH}/libt_cose/cmake" CACHE PATH "t_cose CMake package directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching t_cose
FetchContent_Declare(
	t_cose
	GIT_REPOSITORY ${T_COSE_URL}
	GIT_TAG ${T_COSE_REFSPEC}
	GIT_SHALLOW TRUE

	PATCH_COMMAND git stash
		COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/0001-add-install-definition.patch
		COMMAND git reset HEAD~1

)

# FetchContent_GetProperties exports t_cose_SOURCE_DIR and t_cose_BINARY_DIR variables
FetchContent_GetProperties(t_cose)
if(NOT t_cose_POPULATED)
	message(STATUS "Fetching t_cose")
	FetchContent_Populate(t_cose)
endif()

# Prepare include paths for dependencies that t_codse has on external components
get_target_property(_qcbor_inc qcbor INTERFACE_INCLUDE_DIRECTORIES)
set(_ext_inc_paths
	${_qcbor_inc}
	${PSA_CRYPTO_API_INCLUDE})

if (NOT TCOSE_EXTERNAL_INCLUDE_PATHS STREQUAL "")
	list(APPEND _ext_inc_paths  "${TCOSE_EXTERNAL_INCLUDE_PATHS}")
	unset(TCOSE_EXTERNAL_INCLUDE_PATHS)
endif()

string(REPLACE ";" "\\;" _ext_inc_paths "${_ext_inc_paths}")

# Configure the t_cose library
execute_process(COMMAND
${CMAKE_COMMAND}
	-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
	-Dthirdparty_inc=${_ext_inc_paths}
	-DCMAKE_INSTALL_PREFIX=${T_COSE_INSTALL_PATH}
	-DMBEDTLS=On
	-GUnix\ Makefiles
	${t_cose_SOURCE_DIR}
WORKING_DIRECTORY
	${t_cose_BINARY_DIR}
)

# Build the library
execute_process(COMMAND
		${CMAKE_COMMAND} --build ${t_cose_BINARY_DIR} --parallel ${PROCESSOR_COUNT} --target install
		RESULT_VARIABLE _exec_error
	)
if (_exec_error)
	message(FATAL_ERROR "Build step of t_cose failed with ${_exec_error}.")
endif()

# Create an imported target to have clean abstraction in the build-system.
add_library(t_cose STATIC IMPORTED)
set_property(TARGET t_cose PROPERTY IMPORTED_LOCATION "${T_COSE_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}t_cose${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(TARGET t_cose PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${T_COSE_INSTALL_PATH}/include")

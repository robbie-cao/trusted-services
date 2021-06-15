#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# QCBOR is a library for encoding and decodingg CBOR objects, as per RFC8949
#-------------------------------------------------------------------------------

# External component details
set(QCBOR_URL "https://github.com/laurencelundblade/QCBOR.git" CACHE STRING "qcbor repository URL")
set(QCBOR_REFSPEC "master" CACHE STRING "qcbor git refspec")
set(QCBOR_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/qcbor_install" CACHE PATH "qcbor installation directory")
set(QCBOR_PACKAGE_PATH "${QCBOR_INSTALL_PATH}/libqcbor/cmake" CACHE PATH "qcbor CMake package directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching qcbor
FetchContent_Declare(
	qcbor
	GIT_REPOSITORY ${QCBOR_URL}
	GIT_TAG ${QCBOR_REFSPEC}
	GIT_SHALLOW TRUE

	PATCH_COMMAND git stash
		COMMAND git branch -f bf-patch
		COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/0001-Add-3rd-party-settings.patch ${CMAKE_CURRENT_LIST_DIR}/0002-Add-install-definition.patch
		COMMAND git reset bf-patch
)

# FetchContent_GetProperties exports qcbor_SOURCE_DIR and qcbor_BINARY_DIR variables
FetchContent_GetProperties(qcbor)
if(NOT qcbor_POPULATED)
	message(STATUS "Fetching qcbor")
	FetchContent_Populate(qcbor)
endif()

# Determine floating point configuration
if (TS_NO_FLOAT_HW)
	set(_thirdparty_def -DQCBOR_DISABLE_FLOAT_HW_USE)
endif()

# Configure the qcbor library
if (NOT "${QCBOR_EXTERNAL_INCLUDE_PATHS}" STREQUAL "")
	string(REPLACE ";" "\\;" QCBOR_EXTERNAL_INCLUDE_PATHS "${QCBOR_EXTERNAL_INCLUDE_PATHS}")
	set(QCBOR_EXTRA_OPTION -Dthirdparty_inc=${QCBOR_EXTERNAL_INCLUDE_PATHS})
	unset(QCBOR_EXTERNAL_INCLUDE_PATHS)
else()
	set(QCBOR_EXTRA_OPTION "")
endif()

execute_process(COMMAND
	${CMAKE_COMMAND}
		-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
		-GUnix\ Makefiles
		-Dthirdparty_def=${_thirdparty_def}
		-DCMAKE_INSTALL_PREFIX=${QCBOR_INSTALL_PATH}
		${QCBOR_EXTRA_OPTION}
		${qcbor_SOURCE_DIR}
	WORKING_DIRECTORY
		${qcbor_BINARY_DIR}
)
unset(QCBOR_EXTRA_OPTION)

# Build the library
execute_process(COMMAND
		${CMAKE_COMMAND} --build ${qcbor_BINARY_DIR} -j8
		RESULT_VARIABLE _exec_error
	)
if (_exec_error)
	message(FATAL_ERROR "Build step of qcbor failed with ${_exec_error}.")
endif()

execute_process(COMMAND
		${CMAKE_COMMAND} --install ${qcbor_BINARY_DIR}
		RESULT_VARIABLE _exec_error
	)
if (_exec_error)
	message(FATAL_ERROR "Build step of qcbor failed with ${_exec_error}.")
endif()

# Create an imported target to have clean abstraction in the build-system.
add_library(qcbor STATIC IMPORTED)
set_property(TARGET qcbor PROPERTY IMPORTED_LOCATION "${QCBOR_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}qcbor${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(TARGET qcbor PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${QCBOR_INSTALL_PATH}/include")

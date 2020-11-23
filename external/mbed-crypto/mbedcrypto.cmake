#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MBEDCRYPTO_URL "https://github.com/ARMmbed/mbed-crypto.git" CACHE STRING "mbedcrypto repository URL")
set(MBEDCRYPTO_REFSPEC "mbedcrypto-3.1.0" CACHE STRING "mbedcrypto git refspec")
set(MBEDCRYPTO_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/mbedcrypto_install" CACHE PATH "mbedcrypto installation directory")
set(MBEDCRYPTO_PACKAGE_PATH "${MBEDCRYPTO_INSTALL_PATH}/lib/mbedcrypto/cmake" CACHE PATH "mbedcrypto CMake package directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching mbedcrypto
FetchContent_Declare(
	mbedcrypto
	GIT_REPOSITORY ${MBEDCRYPTO_URL}
	GIT_TAG ${MBEDCRYPTO_REFSPEC}
	GIT_SHALLOW TRUE
)

# FetchContent_GetProperties exports mbedcrypto_SOURCE_DIR and mbedcrypto_BINARY_DIR variables
FetchContent_GetProperties(mbedcrypto)
if(NOT mbedcrypto_POPULATED)
	message(STATUS "Fetching mbedcrypto")
	FetchContent_Populate(mbedcrypto)
endif()

# Convert the include path list to a string. Needed to make parameter passing to
# mbedcrypto build work fine.
string(REPLACE ";" "\\;" MBEDCRYPTO_EXTRA_INCLUDES "${MBEDCRYPTO_EXTRA_INCLUDES}")

#Configure the library
if(NOT CMAKE_CROSSCOMPILING)
	execute_process(COMMAND
		${CMAKE_COMMAND}
			-DENABLE_PROGRAMS=OFF
			-DENABLE_TESTING=OFF
			-DCMAKE_INSTALL_PREFIX=${MBEDCRYPTO_INSTALL_PATH}
			-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
			-Dthirdparty_def=-DMBEDTLS_CONFIG_FILE="${MBEDCRYPTO_CONFIG_FILE}"
			-Dthirdparty_inc=${MBEDCRYPTO_EXTRA_INCLUDES}
			-GUnix\ Makefiles
			${mbedcrypto_SOURCE_DIR}
		WORKING_DIRECTORY
			${mbedcrypto_BINARY_DIR}
		)
else()
	execute_process(COMMAND
		${CMAKE_COMMAND}
			-DENABLE_PROGRAMS=OFF
			-DENABLE_TESTING=OFF
			-DCMAKE_INSTALL_PREFIX=${MBEDCRYPTO_INSTALL_PATH}
			-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
			-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
			-Dthirdparty_def=-DMBEDTLS_CONFIG_FILE="${MBEDCRYPTO_CONFIG_FILE}"
			-Dthirdparty_inc=${MBEDCRYPTO_EXTRA_INCLUDES}
			-GUnix\ Makefiles
			${mbedcrypto_SOURCE_DIR}
		WORKING_DIRECTORY
			${mbedcrypto_BINARY_DIR}
		RESULT_VARIABLE _exec_error
	)

	if (_exec_error)
		message(FATAL_ERROR "Configuration step of mbedcrypto failed with ${_exec_error}.")
	endif()
endif()

#TODO: add dependnecy to generated project on this file!
#TODO: add custom target to rebuild mbedcrypto

#Build the library
execute_process(COMMAND
		${CMAKE_COMMAND} --build ${mbedcrypto_BINARY_DIR} -- install -j8
		RESULT_VARIABLE _exec_error
	)
if (_exec_error)
	message(FATAL_ERROR "Build step of mbedcrypto failed with ${_exec_error}.")
endif()

#Create an imported target to have clean abstraction in the build-system.
add_library(mbedcrypto STATIC IMPORTED)
set_property(TARGET mbedcrypto PROPERTY IMPORTED_LOCATION "${MBEDCRYPTO_INSTALL_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mbedcrypto${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(TARGET mbedcrypto PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${MBEDCRYPTO_INSTALL_PATH}/include")


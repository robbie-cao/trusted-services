#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MBEDTLS_URL "https://github.com/ARMmbed/mbedtls.git"
		CACHE STRING "Mbed TLS repository URL")
set(MBEDTLS_REFSPEC "mbedtls-3.0.0"
		CACHE STRING "Mbed TLS git refspec")
set(MBEDTLS_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/mbedtls-src"
		CACHE PATH "MbedTLS source directory")
set(MBEDTLS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_install"
		CACHE PATH "Mbed TLS installation directory")

find_library(MBEDCRYPTO_LIB_FILE
				NAMES libmbedcrypto.a mbedcrypto.a libmbedcrypto.lib mbedcrypto.lib
				PATHS ${MBEDTLS_INSTALL_DIR}
				PATH_SUFFIXES "lib"
				DOC "Location of mberdrypto library."
				NO_DEFAULT_PATH
)

set(MBEDCRYPTO_LIB_FILE ${MBEDCRYPTO_LIB_FILE})
unset(MBEDCRYPTO_LIB_FILE CACHE)

set(MBEDTLS_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/mbedtls-build")

# Binary not found and it needs to be built.
if (NOT MBEDCRYPTO_LIB_FILE)
	# Determine the number of processes to run while running parallel builds.
	# Pass -DPROCESSOR_COUNT=<n> to cmake to override.
	if(NOT DEFINED PROCESSOR_COUNT)
		include(ProcessorCount)
		ProcessorCount(PROCESSOR_COUNT)
		set(PROCESSOR_COUNT ${PROCESSOR_COUNT}
				CACHE STRING "Number of cores to use for parallel builds.")
	endif()

	# See if the source is available locally
	find_file(MBEDCRYPTO_HEADER_FILE
		NAMES crypto.h
		PATHS ${MBEDTLS_SOURCE_DIR}
		PATH_SUFFIXES "include/psa"
		NO_DEFAULT_PATH
	)
	set(MBEDCRYPTO_HEADER_FILE ${MBEDCRYPTO_HEADER_FILE})
	unset(MBEDCRYPTO_HEADER_FILE CACHE)

	# Source not found, fetch it.
	if (NOT MBEDCRYPTO_HEADER_FILE)
		include(FetchContent)

		# Checking git
		find_program(GIT_COMMAND "git")
		if (NOT GIT_COMMAND)
			message(FATAL_ERROR "Please install git")
		endif()

		# Fetching Mbed TLS
		FetchContent_Declare(
			mbedtls
			SOURCE_DIR ${MBEDTLS_SOURCE_DIR}
			BINARY_DIR ${MBEDTLS_BINARY_DIR}
			GIT_REPOSITORY ${MBEDTLS_URL}
			GIT_TAG ${MBEDTLS_REFSPEC}
			GIT_SHALLOW TRUE
		)

		# FetchContent_GetProperties exports mbedtls_SOURCE_DIR and mbedtls_BINARY_DIR variables
		FetchContent_GetProperties(mbedtls)
		# FetchContent_Populate will fail if the source directory is removed since it will try to
		# do an "update" and not a "populate" action. As a workaround, remove the subbuild directory.
		# Note: this fix assumes, the default subbuild location is used.
		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/_deps/mbedtls-subbuild")

		# If the source directory has been moved, the binary dir must be regenerated from scratch.
		file(REMOVE_RECURSE "${MBEDTLS_BINARY_DIR}")

		if (NOT mbedtls_POPULATED)
			message(STATUS "Fetching Mbed TLS")
			FetchContent_Populate(mbedtls)
		endif()
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MBEDTLS_SOURCE_DIR})
	endif()

	# Build mbedcrypto library

	# Convert the include path list to a string. Needed to make parameter passing to
	# Mbed TLS build work fine.
	string(REPLACE ";" "\\;" MBEDTLS_EXTRA_INCLUDES "${MBEDTLS_EXTRA_INCLUDES}")

	find_package(Python3 REQUIRED COMPONENTS Interpreter)

	#Configure Mbed TLS to build only mbedcrypto lib
	execute_process(COMMAND ${Python3_EXECUTABLE} scripts/config.py crypto WORKING_DIRECTORY ${MBEDTLS_SOURCE_DIR})

	# Advertise Mbed TLS as the provider of the psa crypto API
	set(PSA_CRYPTO_API_INCLUDE "${MBEDTLS_INSTALL_DIR}/include" CACHE STRING "PSA Crypto API include path")

	#Configure the library
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env CROSS_COMPILE=${CROSS_COMPILE}
			${CMAKE_COMMAND}
				-DENABLE_PROGRAMS=OFF
				-DENABLE_TESTING=OFF
				-DUNSAFE_BUILD=ON
				-DCMAKE_INSTALL_PREFIX=${MBEDTLS_INSTALL_DIR}
				-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
				-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
				-DEXTERNAL_DEFINITIONS=-DMBEDTLS_USER_CONFIG_FILE="${MBEDTLS_USER_CONFIG_FILE}"
				-DEXTERNAL_INCLUDE_PATHS=${MBEDTLS_EXTRA_INCLUDES}
				-GUnix\ Makefiles
				${MBEDTLS_SOURCE_DIR}
		WORKING_DIRECTORY
			${MBEDTLS_BINARY_DIR}
		RESULT_VARIABLE _exec_error
	)

	if (_exec_error)
		message(FATAL_ERROR "Configuration step of Mbed TLS failed with ${_exec_error}.")
	endif()

	#Build the library
	execute_process(COMMAND
			${CMAKE_COMMAND} --build ${MBEDTLS_BINARY_DIR} --parallel ${PROCESSOR_COUNT} --target install
			RESULT_VARIABLE _exec_error
		)

	if (_exec_error)
		message(FATAL_ERROR "Build step of Mbed TLS failed with ${_exec_error}.")
	endif()

	set(MBEDCRYPTO_LIB_FILE "${MBEDTLS_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mbedcrypto${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

#Create an imported target to have clean abstraction in the build-system.
add_library(mbedcrypto STATIC IMPORTED)
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MBEDCRYPTO_LIB_FILE})
set_property(TARGET mbedcrypto PROPERTY IMPORTED_LOCATION ${MBEDCRYPTO_LIB_FILE})
set_property(TARGET mbedcrypto PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${MBEDTLS_INSTALL_DIR}/include")

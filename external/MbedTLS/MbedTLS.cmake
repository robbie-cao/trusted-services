#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MBEDTLS_URL "https://github.com/Mbed-TLS/mbedtls.git"
		CACHE STRING "Mbed TLS repository URL")
set(MBEDTLS_REFSPEC "mbedtls-3.3.0"
		CACHE STRING "Mbed TLS git refspec")
set(MBEDTLS_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/mbedtls-src"
		CACHE PATH "MbedTLS source directory")
set(MBEDTLS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_install"
		CACHE PATH "Mbed TLS installation directory")
set(MBEDTLS_BUILD_TYPE "Release" CACHE STRING "Mbed TLS build type")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Mbed TLS has a custom config script that must be ran before invoking CMake.
# This script configures which components of the project will get built, in our
# use case only mbedcrypto is necessary. LazyFetch has a PATCH_COMMAND option
# that was intended to be used for patching the repo after fetch, but before
# running CMake. However, it can be "misused" in this case to run the Mbed TLS
# config script.
set(GIT_OPTIONS
	GIT_REPOSITORY ${MBEDTLS_URL}
	GIT_TAG ${MBEDTLS_REFSPEC}
	GIT_SHALLOW FALSE
	PATCH_COMMAND ${Python3_EXECUTABLE} scripts/config.py crypto
)

# Only pass libc settings to Mbed TLS if needed. For environments where the standard
# library is not overridden, this is not needed.
if(TARGET stdlib::c)
	include(${TS_ROOT}/tools/cmake/common/PropertyCopy.cmake)
	# Save libc settings
	save_interface_target_properties(TGT stdlib::c PREFIX LIBC)
	# Translate libc settings to CMake code fragment. Will be inserted into
	# mbedtls-init-cache.cmake.in when LazyFetch configures the file.
	translate_interface_target_properties(PREFIX LIBC RES _cmake_fragment)
	unset_saved_properties(LIBC)
endif()

include(${TS_ROOT}/tools/cmake/common/LazyFetch.cmake REQUIRED)
LazyFetch_MakeAvailable(DEP_NAME MbedTLS
	FETCH_OPTIONS ${GIT_OPTIONS}
	INSTALL_DIR ${MBEDTLS_INSTALL_DIR}
	PACKAGE_DIR ${MBEDTLS_INSTALL_DIR}/cmake
	CACHE_FILE "${TS_ROOT}/external/MbedTLS/mbedtls-init-cache.cmake.in"
)
unset(_cmake_fragment)

# Link the libraries created by Mbed TLS to libc if needed. For environments where the standard
# library is not overridden, this is not needed.
if(TARGET stdlib::c)
	foreach(_mbedtls_tgt IN ITEMS "MbedTLS::mbedcrypto" "MbedTLS::mbedx509" "MbedTLS::mbedtls")
		target_link_libraries(${_mbedtls_tgt} INTERFACE stdlib::c)
	endforeach()
	unset(_mbedtls_tgt)
endif()

# Advertise Mbed TLS as the provider of the PSA Crypto API
set(PSA_CRYPTO_API_INCLUDE "${MBEDTLS_INSTALL_DIR}/include"
		CACHE STRING "PSA Crypto API include path")

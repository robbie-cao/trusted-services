#-------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'platform-inspect' for
#  different environments.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Use libts for locating and accessing trusted services. An appropriate version
#  of libts will be imported for the environment in which platform-inspect is
#  built.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(platform-inspect PRIVATE libts::ts)

#-------------------------------------------------------------------------------
#  Components that are common across all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "platform-inspect"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/platform-inspect"
		"components/common/tlv"
		"components/common/cbor_dump"
		"components/service/common/client"
		"components/service/common/include"
		"components/service/attestation/include"
		"components/service/attestation/client/psa"
		"components/service/attestation/client/provision"
		"components/service/attestation/reporter/dump/raw"
		"components/service/attestation/reporter/dump/pretty"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# MbedTLS provides libmbedcrypto
set(MBEDTLS_USER_CONFIG_FILE "${TS_ROOT}/external/MbedTLS/config/crypto_posix.h"
	CACHE STRING "Configuration file for mbedcrypto")
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(platform-inspect PRIVATE MbedTLS::mbedcrypto)

# Use Mbed TLS to provide the psa crypto api interface files
set(PSA_CRYPTO_API_INCLUDE "${MBEDTLS_PUBLIC_INCLUDE_PATH}")

# Qcbor
include(${TS_ROOT}/external/qcbor/qcbor.cmake)

# t_cose
include(${TS_ROOT}/external/t_cose/t_cose.cmake)
# Ensure correct order of libraries on the command line of LD. t_cose depends on qcbor thus
# qcbor must come later.
target_link_libraries(platform-inspect PRIVATE t_cose qcbor)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS platform-inspect RUNTIME DESTINATION ${TS_ENV}/bin)

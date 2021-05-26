#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
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
#  of libts will be imported for the enviroment in which platform-inspect is
#  built.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(platform-inspect PRIVATE libts)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "platform-inspect"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/platform-inspect"
		"components/common/tlv"
		"components/common/cbor_dump"
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

# Configuration for mbedcrypto
set(MBEDTLS_USER_CONFIG_FILE
	"${TS_ROOT}/components/service/crypto/client/cpp/config_mbedtls_user.h"
	CACHE STRING "Configuration file for mbedcrypto")

# Mbed TLS provides libmbedcrypto
include(../../../external/MbedTLS/MbedTLS.cmake)
target_link_libraries(platform-inspect PRIVATE mbedcrypto)

# Qcbor
include(${TS_ROOT}/external/qcbor/qcbor.cmake)
target_link_libraries(platform-inspect PRIVATE qcbor)

# t_cose
include(${TS_ROOT}/external/t_cose/t_cose.cmake)
target_link_libraries(platform-inspect PRIVATE t_cose)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS platform-inspect RUNTIME DESTINATION ${TS_ENV}/bin)

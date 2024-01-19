#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'ts-demo' for
#  different environments.  Demonstrates use of trusted services by a
#  client application.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Use libts for locating and accessing services. An appropriate version of
#  libts will be imported for the environment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(ts-demo PRIVATE libts::ts)

#-------------------------------------------------------------------------------
#  Common main for all deployments
#
#-------------------------------------------------------------------------------
target_sources(ts-demo PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ts-demo.cpp"
)

#-------------------------------------------------------------------------------
#  Components that are common across all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts-demo"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/ts-demo"
		"components/common/tlv"
		"components/service/common/include"
		"components/service/common/client"
		"components/service/crypto/client/cpp"
		"components/service/crypto/client/cpp/protocol/packed-c"
		"protocols/service/crypto/packed-c"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# MbedTLS provides libmbedcrypto
set(MBEDTLS_USER_CONFIG_FILE "${TS_ROOT}/external/MbedTLS/config/crypto_posix.h"
	CACHE STRING "Configuration file for mbedcrypto")
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(ts-demo PRIVATE MbedTLS::mbedcrypto)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS ts-demo RUNTIME DESTINATION ${TS_ENV}/bin)

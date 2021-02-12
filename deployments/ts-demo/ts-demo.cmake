#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
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
#  libts will be imported for the enviroment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(ts-demo PRIVATE libts)

#-------------------------------------------------------------------------------
#  Common main for all deployments
#
#-------------------------------------------------------------------------------
target_sources(ts-demo PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ts-demo.cpp"
)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts-demo"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/ts-demo"
		"components/service/crypto/client/cpp"
		"components/service/common/serializer/protobuf"
		"protocols/service/crypto/protobuf"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(ts-demo PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "ts-demo" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbedcrypto
include(${TS_ROOT}/external/mbed-crypto/mbedcrypto.cmake)
target_link_libraries(ts-demo PRIVATE mbedcrypto)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS ts-demo RUNTIME DESTINATION ${TS_ENV}/bin)

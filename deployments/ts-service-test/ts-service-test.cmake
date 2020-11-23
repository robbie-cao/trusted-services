#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'ts-service-test' for
#  different environments.  Used for running end-to-end service-level tests
#  where test cases excerise trusted service client interfaces.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Use libts for locating and accessing services. An appropriate version of
#  libts will be imported for the enviroment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(ts-service-test PRIVATE libts)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts-service-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/test-runner"
		"components/service/crypto/test/service"
		"components/service/crypto/client/cpp"
		"components/service/common/serializer/protobuf"
		"protocols/service/crypto/protobuf"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# CppUTest
include(${TS_ROOT}/external/CppUTest/CppUTest.cmake)
target_link_libraries(ts-service-test PRIVATE CppUTest)

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(ts-service-test PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "ts-service-test" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbedcrypto
include(${TS_ROOT}/external/mbed-crypto/mbedcrypto.cmake)
target_link_libraries(ts-service-test PRIVATE mbedcrypto)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS ts-service-test RUNTIME DESTINATION bin)
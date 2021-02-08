#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the libts deployment for linux-pc
#
#  Used for building the libts library for the linux-pc enviroment.  Also
#  builds the libts-test executable that acts as a client for the services
#  accessed via libts.  For the linux-pc deployment, libts contains
#  standalone versions of a set of trusted services to support client
#  application development in a native PC environment.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/linux-pc/env_shared_lib.cmake)
project(trusted-services LANGUAGES CXX C)
add_library(ts SHARED)
target_include_directories(ts PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the linux-pc environment.
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/rpc/direct"
		"components/common/tlv"
		"components/service/common"
		"components/service/common/serializer/protobuf"
		"components/service/common/provider"
		"components/service/locator/standalone"
		"components/service/locator/standalone/services/crypto"
		"components/service/locator/standalone/services/test-runner"
		"components/service/crypto/provider/mbedcrypto"
		"components/service/crypto/provider/mbedcrypto/trng_adapter/linux"
		"components/service/crypto/provider/serializer/protobuf"
		"components/service/crypto/provider/serializer/packed-c"
		"components/service/secure_storage/client/psa"
		"components/service/secure_storage/provider/secure_flash_store"
		"components/service/secure_storage/provider/secure_flash_store/flash_fs"
		"components/service/secure_storage/provider/secure_flash_store/flash"
		"components/service/test_runner/provider"
		"components/service/test_runner/provider/serializer/packed-c"
		"components/service/test_runner/provider/backend/mock"
		"components/service/test_runner/provider/backend/simple_c"
		"protocols/rpc/common/packed-c"
		"protocols/service/crypto/packed-c"
		"protocols/service/crypto/protobuf"
		"protocols/service/secure_storage/packed-c"
)

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  libts
#
#-------------------------------------------------------------------------------
include(../libts.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Components used by libts from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(ts PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "ts" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbedcrypto
include(${TS_ROOT}/external/mbed-crypto/mbedcrypto.cmake)
target_link_libraries(ts PRIVATE mbedcrypto)

#-------------------------------------------------------------------------------
#  Test executable (libts-test) for testing libts static library
#
#-------------------------------------------------------------------------------
add_executable(libts-test)
target_include_directories(libts-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
target_link_libraries(libts-test PRIVATE "-Wl,--whole-archive" ts "-Wl,--no-whole-archive")

add_components(
	TARGET "libts-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/test-runner"
		"components/common/tlv"
		"components/service/crypto/test/service"
		"components/service/crypto/test/service/protobuf"
		"components/service/crypto/test/service/packed-c"
		"components/service/crypto/client/cpp"
		"components/service/crypto/client/cpp/protobuf"
		"components/service/crypto/client/cpp/packed-c"
		"components/service/common/serializer/protobuf"
		"protocols/service/crypto/protobuf"
		"protocols/service/crypto/packed-c"
)

#-------------------------------------------------------------------------------
#  Components used by libts-test from external projects
#
#-------------------------------------------------------------------------------

# CppUTest
include(${TS_ROOT}/external/CppUTest/CppUTest.cmake)
target_link_libraries(libts-test PRIVATE CppUTest)

# Nanopb
target_link_libraries(libts-test PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "libts-test" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbedcrypto
target_link_libraries(libts-test PRIVATE mbedcrypto)

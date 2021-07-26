#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../../deployment.cmake REQUIRED)
include(../../psa-api-test-config.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the psa-api-test deployment for arm-linux
#
#  Used for building and running psa arch tests on an Arm based Linux device
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/arm-linux/env.cmake)
project(trusted-services LANGUAGES CXX C)
add_executable(psa-api-test)
target_include_directories(psa-api-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  psa-api-test/initial_attestation
#-------------------------------------------------------------------------------
include(../iat-api-test.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Linker option to enable repeated searches for undefined references.
#  Required to resolve dependencies between arch-test and mbedcrypto libraries.
#-------------------------------------------------------------------------------
target_link_options(psa-api-test PRIVATE
	-Wl,--start-group
	)

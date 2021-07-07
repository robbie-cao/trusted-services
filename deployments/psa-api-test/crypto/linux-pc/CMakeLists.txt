#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../../deployment.cmake REQUIRED)

# Prevents symbols in the psa-api-test executable overriding symbols with
# with same name in libts during dyanmic linking performed by the program
# loader.  This avoid psa crypto api symbols provided by the mbedcrypto
# library from being overridden by the same symbols in the psa-api-test
# executable.
set(CMAKE_C_VISIBILITY_PRESET hidden)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the psa-api-test deployment for linux-pc
#
#  Used for building and running psa arch tests in a native PC enviroment.
#  Tests can be run by running the built executable called "psa-api-test"
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/linux-pc/env.cmake)
project(trusted-services LANGUAGES CXX C)
add_executable(psa-api-test)
target_include_directories(psa-api-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  psa-api-test/crypto
#-------------------------------------------------------------------------------
include(../crypto-api-test.cmake REQUIRED)

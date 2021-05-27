#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the ts-arch-test deployment for linux-pc
#
#  Used for building and running psa arch tests in a native PC enviroment.
#  Tests can be run by running the built executable called "ts-arch-test"
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/linux-pc/env.cmake)
project(trusted-services LANGUAGES CXX C)
add_executable(ts-arch-test)
target_include_directories(ts-arch-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  ts-arch-test/initial_attestation
#-------------------------------------------------------------------------------
include(../iat-arch-test.cmake REQUIRED)

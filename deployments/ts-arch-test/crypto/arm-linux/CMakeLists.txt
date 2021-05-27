#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the ts-arch-test deployment for arm-linux
#
#  Used for building and running psa arch tests on an Arm based Linux device
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/arm-linux/env.cmake)
project(trusted-services LANGUAGES CXX C)
add_executable(ts-arch-test)
target_include_directories(ts-arch-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  ts-arch-test/crypto
#-------------------------------------------------------------------------------
include(../crypto-arch-test.cmake REQUIRED)

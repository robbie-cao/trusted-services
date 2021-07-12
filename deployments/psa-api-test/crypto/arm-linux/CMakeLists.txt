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
#  psa-api-test/crypto
#-------------------------------------------------------------------------------
include(../crypto-api-test.cmake REQUIRED)

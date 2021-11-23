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
#  The CMakeLists.txt for building the psa-api-test/internal_trusted_storage
#  deployment for linux-pc
#
#  Used for building and running psa arch tests in a native PC enviroment.
#  Tests can be run by running the built executable called "psa-its-api-test".
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/linux-pc/env.cmake)
project(psa-its-api-test LANGUAGES CXX C)
add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  psa-api-test/internal_trusted_storage
#-------------------------------------------------------------------------------
include(../its-api-test.cmake REQUIRED)

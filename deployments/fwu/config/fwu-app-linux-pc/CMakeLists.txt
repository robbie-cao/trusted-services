#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(../../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
# The CMakeLists.txt for building the fwu deployment for linux-pc
#
# This configuration builds the FWU update agent into a command-line app
# that can be used to apply updates to disk image files.
#-------------------------------------------------------------------------------
project(trusted-services LANGUAGES CXX C)
add_executable(fwu)
target_include_directories(fwu PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Configure trace output for command-line app
#
#-------------------------------------------------------------------------------
set(TRACE_PREFIX "FWU" CACHE STRING "Trace prefix")
set(TRACE_LEVEL "TRACE_LEVEL_DEBUG" CACHE STRING "Trace level")

#-------------------------------------------------------------------------------
#  Deployment specific build configuration
#
#-------------------------------------------------------------------------------

# Scale number of partitions for pretty complicated fw images
target_compile_definitions("fwu" PRIVATE
    PARTITIONED_BLOCK_STORE_MAX_PARTITIONS=24)

#-------------------------------------------------------------------------------
# This configuration builds for linux-pc
#
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/linux-pc/env.cmake)

#-------------------------------------------------------------------------------
#  External project source-level dependencies
#
#-------------------------------------------------------------------------------
include(${TS_ROOT}/external/tf_a/tf-a.cmake)
add_tfa_dependency(TARGET "fwu")

#-------------------------------------------------------------------------------
#  Deployment specific components
#
#-------------------------------------------------------------------------------
include(../../env/posix/fwu-posix.cmake REQUIRED)
include(../../fwu.cmake REQUIRED)
include(../../infra/file-block-store.cmake REQUIRED)

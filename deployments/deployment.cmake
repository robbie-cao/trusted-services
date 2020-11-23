#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
  The base deployment CMake file
  ------------------------------

  Contains common CMake definitions that are used by concrete deployments.
  This file should be included first by a concrete deployment's CMakeLists.txt.
#]===]

# Sets TS-ROOT which is used as the reference directory for everything contained within the project
get_filename_component(TS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE CACHE PATH "Trusted Services root directory.")

# Replicate TS_ROOT as environment variable to allow access from child CMake contexts
set(ENV{TS_ROOT} "${TS_ROOT}")

# Common utilities used by the build system
include(${TS_ROOT}/tools/cmake/common/Utils.cmake REQUIRED)
include(${TS_ROOT}/tools/cmake/common/AddComponents.cmake REQUIRED)

# Check build environment requirements are met
ts_verify_build_env()

# Project wide include directories
set(TOP_LEVEL_INCLUDE_DIRS
  "${TS_ROOT}"
  "${TS_ROOT}/components"
  )

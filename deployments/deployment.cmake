#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
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
include(${TS_ROOT}/tools/cmake/common/AddPlatform.cmake REQUIRED)

# Check build environment requirements are met
ts_verify_build_env()

# Project wide include directories
set(TOP_LEVEL_INCLUDE_DIRS
  "${TS_ROOT}"
  "${TS_ROOT}/components"
  )

# Set platform provider root default to use if no commandline variable value has been specified.
# The root path may be specified to allow an external project to provide platform definitions.
if (DEFINED ENV{TS_PLATFORM_ROOT})
  set(_default_platform_root ENV{TS_PLATFORM_ROOT})
else()
  set(_default_platform_root "${TS_ROOT}/platform/providers")
endif()
set(TS_PLATFORM_ROOT ${_default_platform_root} CACHE STRING "Platform provider path")

# Set the default platform to use if no explict platform has been specified on the cmake commandline.
if (DEFINED ENV{TS_PLATFORM})
  set(_default_platform ENV{TS_PLATFORM})
else()
  set(_default_platform "ts/vanilla")
endif()
set(TS_PLATFORM ${_default_platform} CACHE STRING "Selected platform")

# Custom property for defining platform feature dependencies based on components used in a deployment
define_property(TARGET PROPERTY TS_PLATFORM_DRIVER_DEPENDENCIES
  BRIEF_DOCS "List of platform driver interfaces used for a deployment."
  FULL_DOCS "Used by the platform specific builder to specify a configuration for the built platform components."
  )
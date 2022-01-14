#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.14)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the newlib deployment for opteesp
#
#  Can be used to build the newlib library, which can be used to build SPs.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)

project(newlib C)

# This is a dummy library not intended to be compiled ever.
add_library(dummy EXCLUDE_FROM_ALL)
set(TGT dummy)
# Build newlib as an external component.
include(${TS_ROOT}/external/newlib/newlib.cmake)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()

# export the newlib install content from CMAKE_BINARY_DIR.
install(DIRECTORY ${NEWLIB_INSTALL_DIR}/ DESTINATION ${TS_ENV}/newlib)

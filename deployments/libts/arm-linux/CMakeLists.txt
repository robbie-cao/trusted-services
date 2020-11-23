#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the libts deployment for arm-linux
#
#  Used for building the libts library for the arm-linux enviroment.  Used for
#  locating and accessing services from a Linux userspace client.  Service
#  instances can be located in any supported secure processing enviroment.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/arm-linux/env_shared_lib.cmake)
project(trusted-services LANGUAGES CXX C)
add_library(ts SHARED)
target_include_directories(ts PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the arm-linux environment.
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/rpc/ffarpc/caller/linux"
		"components/service/locator/linux"
		"components/service/locator/linux/ffa"
		"components/common/uuid"
)

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  libts
#
#-------------------------------------------------------------------------------
include(../libts.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Define library options and dependencies.
#
#-------------------------------------------------------------------------------
env_set_link_options(TGT ts)
target_link_libraries(ts PRIVATE gcc)
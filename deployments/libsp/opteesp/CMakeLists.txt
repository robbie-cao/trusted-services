#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the libsp deployment for opteesp
#
#  Used for building the libsp library that provides FFA related functions
#  for applications deployed in a secure partition.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)

file(READ "${CMAKE_CURRENT_LIST_DIR}/version.txt" LIBSP_VERSION)
project(trusted-services
			VERSION
				${LIBSP_VERSION}
			 LANGUAGES
				C ASM
		)

add_library(sp STATIC)

add_components(TARGET sp
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/messaging/ffa/libsp
		components/common/utils
)

# Include newlib into the build, but do no add SP executable specific files to libsp.
# Use a dummy target which will never be built.
add_library(dummy EXCLUDE_FROM_ALL)
add_components(TARGET dummy
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		environments/opteesp
)

# Get libc specific settings from newlib.
target_link_libraries(sp PRIVATE stdlib::c)

target_compile_definitions(sp PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(sp PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		-std=c99
	)
endif()

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()

install(
	TARGETS
		sp
	EXPORT
		LibspTargets
	ARCHIVE	DESTINATION
		${TS_ENV}/lib
	PUBLIC_HEADER DESTINATION
		${TS_ENV}/include
	COMPONENT
		libsp
)

### Create a config file package.
set(ConfigPackageLocation ${TS_ENV}/lib/cmake/libsp)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/LibspConfigVersion.cmake"
	VERSION "1.0.0"
	COMPATIBILITY SameMajorVersion
)

# Create targets file.
export(
	EXPORT
		LibspTargets
	FILE
		"${CMAKE_CURRENT_BINARY_DIR}/LibspTargets.cmake"
	NAMESPACE
		libsp::
)

# Finalize config file.
configure_package_config_file(
		LibspConfig.cmake.in
		"${CMAKE_CURRENT_BINARY_DIR}/LibspConfig.cmake"
	PATH_VARS

	INSTALL_DESTINATION
		${ConfigPackageLocation}
)

install(
	EXPORT
		LibspTargets
	FILE
		LibspTargets.cmake
	NAMESPACE
		libsp::
	DESTINATION
		${ConfigPackageLocation}
	COMPONENT
		libsp
)

# install config and version files
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/LibspConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/LibspConfigVersion.cmake"
	DESTINATION
		${ConfigPackageLocation}
	COMPONENT
		libsp
)

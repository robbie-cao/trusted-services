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

version_semver_read(FILE "${CMAKE_CURRENT_LIST_DIR}/version.txt"
					MAJOR _major MINOR _minor PATCH _patch)
set(LIBSP_VERSION "${_major}.${_minor}.${_patch}")
project(trusted-services
			VERSION
				${LIBSP_VERSION}
			 LANGUAGES
				C ASM
		)
unset(_major)
unset(_minor)
unset(_patch)

add_library(sp STATIC)

add_components(TARGET sp
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/messaging/ffa/libsp
		components/common/utils
)

# Include newlib into the build, but do no add SP executable specific files to libsp.
# Use a dummy target which will never be built.
function(get_newlib_compile_flags)
	# Environment specific files require this variable to be set.
	# Set it to a dummy value as we are not going to build these anyways.
	# The namespace of the function will ensure globas setting is not affected.
	set(SP_HEAP_SIZE 4096)
	add_library(dummy EXCLUDE_FROM_ALL)
	add_components(TARGET dummy
		BASE_DIR ${TS_ROOT}
		COMPONENTS
			environments/opteesp
	)
endfunction()

get_newlib_compile_flags()

# Get libc specific settings from newlib.
target_link_libraries(sp PRIVATE stdlib::c)

target_compile_definitions(sp PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(sp PRIVATE
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
	VERSION "${LIBSP_VERSION}"
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

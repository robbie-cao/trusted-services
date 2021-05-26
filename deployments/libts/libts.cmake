#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'libts' for
#  different environments.  libts provides a client interface for locating
#  service instances and establishing RPC sessions for using services.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Common API version implemented by all libts deployments
#-------------------------------------------------------------------------------
version_semver_read(FILE "${CMAKE_CURRENT_LIST_DIR}/version.txt"
					MAJOR _major MINOR _minor PATCH _patch)
set_target_properties(ts PROPERTIES VERSION "${_major}.${_minor}.${_patch}")
set_target_properties(ts PROPERTIES SOVERSION "${_major}")
unset(_major)
unset(_minor)
unset(_patch)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/rpc/common/caller"
		"components/rpc/common/interface"
		"components/service/locator"
		"components/service/locator/interface"
)

#-------------------------------------------------------------------------------
#  Define public interfaces for library
#
#-------------------------------------------------------------------------------

# Enable exporting interface symbols for library public interface
target_compile_definitions(ts PRIVATE
	EXPORT_PUBLIC_INTERFACE_RPC_CALLER
	EXPORT_PUBLIC_INTERFACE_SERVICE_LOCATOR
)

#-------------------------------------------------------------------------------
#  Export the library and the corresponding public interface header files
#
#-------------------------------------------------------------------------------
include(${TS_ROOT}/tools/cmake/common/ExportLibrary.cmake REQUIRED)

# Select public header files to export
get_property(_rpc_caller_public_header_files TARGET ts
	PROPERTY RPC_CALLER_PUBLIC_HEADER_FILES
)

get_property(_service_locator_public_header_files TARGET ts
	PROPERTY SERVICE_LOCATOR_PUBLIC_HEADER_FILES
)

# Exports library information in preparation for install
export_library(
	TARGET "ts"
	LIB_NAME "libts"
	INTERFACE_FILES
		${_rpc_caller_public_header_files}
		${_service_locator_public_header_files}
)

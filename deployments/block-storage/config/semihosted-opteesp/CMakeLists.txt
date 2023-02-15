#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

#-------------------------------------------------------------------------------
# Set default platform.
#
#-------------------------------------------------------------------------------
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")

include(../../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the block_storage deployment for opteesp
#
#  Builds the block_storage service provider for running in an SEL0 secure partition
#  hosted by OPTEE in the role of SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(block-storage)
target_include_directories(block-storage PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID_CANON "63646e80-eb52-462f-ac4f-8cdf3987519c")
set(SP_HEAP_SIZE "120 * 1024" CACHE STRING "SP heap size in bytes")
set(TRACE_PREFIX "BLOCK" CACHE STRING "Trace prefix")
include(${TS_ROOT}/tools/cmake/common/TargetCompileDefinitions.cmake)
set_target_uuids(
	SP_UUID ${SP_UUID_CANON}
	SP_NAME "block-storage"
)

target_include_directories(block-storage PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
)

#-------------------------------------------------------------------------------
#  External project source-level dependencies
#
#-------------------------------------------------------------------------------
include(${TS_ROOT}/external/tf_a/tf-a.cmake)
add_tfa_dependency(TARGET "block-storage")

#-------------------------------------------------------------------------------
#  Deployment specific components. This deployment uses an infrastructure that
#  that provides NV storage backed by a semihosted file residing in a host
#  device filesystem.
#-------------------------------------------------------------------------------
add_components(TARGET "block-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"environments/opteesp"
)

include(../../env/commonsp/block_storage_sp.cmake REQUIRED)
include(../../block-storage.cmake REQUIRED)
include(../../infra/semihosted.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET "block-storage")

#-------------------------------------------------------------------------------
#  Deployment specific build options
#-------------------------------------------------------------------------------
target_compile_definitions(block-storage PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(block-storage PRIVATE
		-std=c99
	)

endif()

compiler_generate_stripped_elf(TARGET block-storage NAME "${SP_UUID_CANON}.stripped.elf" RES STRIPPED_ELF)

#-------------------------------------------------------------------------------
#  Deployment specific install options
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()

install(TARGETS block-storage
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake)
export_sp(
	SP_UUID_CANON ${SP_UUID_CANON}
	SP_UUID_LE ${SP_UUID_LE}
	SP_NAME "block-storage"
	MK_IN ${TS_ROOT}/environments/opteesp/sp.mk.in
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_block-storage.dts.in
	JSON_IN ${TS_ROOT}/environments/opteesp/sp_pkg.json.in
)

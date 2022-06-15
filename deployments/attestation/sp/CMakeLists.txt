#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Set default platform.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the attestation deployment for generic sp
#  environment.
#
#  Builds the attestation service provider for running in an SEL0 secure
#  partition hosted by any SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/sp/env.cmake)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
project(trusted-services LANGUAGES C ASM)
add_executable(attestation)
target_include_directories(attestation PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_NAME "attestation")
set(SP_UUID_CANON "a1baf155-8876-4695-8f7c-54955e8db974")
set(SP_UUID_LE "0x55f1baa1 0x95467688 0x95547c8f 0x74b98d5e")
set(TRACE_PREFIX "ATT" CACHE STRING "Trace prefix")
set(SP_STACK_SIZE "64 * 1024" CACHE STRING "Stack size")
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "Heap size")

#-------------------------------------------------------------------------------
#  Default deployment specific configuration
#
#-------------------------------------------------------------------------------
set(TS_NO_FLOAT_HW ON)

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the sp environment.
#
#-------------------------------------------------------------------------------
add_components(TARGET "attestation"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		environments/sp
)

include(../attestation.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET "attestation")

target_compile_definitions(attestation PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(attestation PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-std=c99
	)

	# Options for GCC that control linking
	target_link_options(attestation PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(attestation PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
	)
endif()

compiler_generate_binary_output(TARGET attestation NAME "${SP_UUID_CANON}.bin" SP_BINARY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP_UUID_CANON}.bin DESTINATION ${TS_ENV}/bin)

include(${TS_ROOT}/tools/cmake/common/ExportMemoryRegionsToManifest.cmake REQUIRED)
export_memory_regions_to_manifest(TARGET attestation NAME "${SP_UUID_CANON}_memory_regions.dtsi" RES EXPORT_MEMORY_REGIONS_DTSI)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS attestation
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake)
export_sp(
	SP_UUID_CANON ${SP_UUID_CANON}
	SP_UUID_LE ${SP_UUID_LE}
	SP_NAME ${SP_NAME}
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_${SP_NAME}.dts.in
	DTS_MEM_REGIONS ${SP_UUID_CANON}_memory_regions.dtsi
	JSON_IN ${TS_ROOT}/environments/sp/sp_pkg.json.in
)

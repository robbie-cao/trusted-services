#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Set default platform.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")
include(../../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the smm-gateway deployment for generic sp
#  environment.
#
#  Provides a service provider for UEFI SMM services that are delegated to
#  other trusted service providers.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/sp/env.cmake)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
project(trusted-services LANGUAGES C ASM)
add_executable(smm-gateway)
target_include_directories(smm-gateway PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_NAME "smm-gateway")
set(SP_UUID_CANON "ed32d533-99e6-4209-9cc0-2d72cdd998a7")
set(SP_UUID_LE "0x33d532ed 0x0942e699 0x722dc09c 0xa798d9cd")
set(TRACE_PREFIX "SMMGW" CACHE STRING "Trace prefix")
set(SP_STACK_SIZE "64 * 1024" CACHE STRING "Stack size")
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "Heap size")

# Setting the MM communication buffer parameters
set(MM_COMM_BUFFER_ADDRESS "0x00000008 0x81000000" CACHE STRING "Address of MM communicte buffer in 64 bit DTS format")
set(MM_COMM_BUFFER_PAGE_COUNT "8" CACHE STRING "Size of the MM communicate buffer in 4k pages")

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the opteesp environment.
#
#-------------------------------------------------------------------------------
add_components(TARGET "smm-gateway"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		environments/sp
)

include(../../env/commonsp/smm_gateway_sp.cmake REQUIRED)
include(../../infra/psa-varstore.cmake REQUIRED)
include(../../smm-gateway.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET "smm-gateway")

#-------------------------------------------------------------------------------
#  Deployment specific build options
#-------------------------------------------------------------------------------
target_compile_definitions(smm-gateway PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(smm-gateway PRIVATE
		-std=c99
	)

endif()

compiler_generate_binary_output(TARGET smm-gateway NAME "${SP_UUID_CANON}.bin" SP_BINARY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP_UUID_CANON}.bin DESTINATION ${TS_ENV}/bin)

include(${TS_ROOT}/tools/cmake/common/ExportMemoryRegionsToManifest.cmake REQUIRED)
export_memory_regions_to_manifest(TARGET smm-gateway NAME "${SP_UUID_CANON}_memory_regions.dtsi" RES EXPORT_MEMORY_REGIONS_DTSI)

#-------------------------------------------------------------------------------
#  Deployment specific install options
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS smm-gateway
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake REQUIRED)
export_sp(
	SP_UUID_CANON ${SP_UUID_CANON}
	SP_UUID_LE ${SP_UUID_LE}
	SP_NAME ${SP_NAME}
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_${SP_NAME}.dts.in
	DTS_MEM_REGIONS ${SP_UUID_CANON}_memory_regions.dtsi
	JSON_IN ${TS_ROOT}/environments/sp/sp_pkg.json.in
)

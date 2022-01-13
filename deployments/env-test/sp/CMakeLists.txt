#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18)

# Set default platform.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the env-test deployment for generic sp
#  environment.
#
#  Builds the test_runner service provider for running in an SEL0 secure
#  partition hosted by any SPM. Environment tests are added and CppUnit test
#  cases.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/sp/env.cmake)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
project(trusted-services LANGUAGES C ASM)
add_executable(env-test)
target_include_directories(env-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_NAME "env-test")
set(SP_UUID "33c75baf-ac6a-4fe4-8ac7-e9909bee2d17")
set(TRACE_PREFIX "ENVTEST" CACHE STRING "Trace prefix")
set(SP_STACK_SIZE "64 * 1024" CACHE STRING "Stack size")
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "Heap size")

add_components(TARGET "env-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/trace"
		"components/common/utils"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint"
		"components/config/test/sp"
		"environments/sp"
)

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  env-test
#
#-------------------------------------------------------------------------------
include(../env-test.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET env-test)

if(CMAKE_CROSSCOMPILING)
	target_link_libraries(env-test PRIVATE stdc++ gcc m)
endif()

#################################################################

target_compile_definitions(env-test PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(env-test PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		$<$<COMPILE_LANGUAGE:C>:-std=c99>
		$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	)

	# Options for GCC that control linking
	target_link_options(env-test PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(env-test PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
	)
endif()

compiler_generate_binary_output(TARGET env-test NAME "${SP_UUID}.bin" SP_BINARY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP_UUID}.bin DESTINATION ${TS_ENV}/bin)

include(${TS_ROOT}/tools/cmake/common/ExportMemoryRegionsToManifest.cmake REQUIRED)
export_memory_regions_to_manifest(TARGET env-test NAME "${SP_UUID}_memory_regions.dtsi" RES EXPORT_MEMORY_REGIONS_DTSI)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS env-test
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake REQUIRED)
export_sp(
	SP_UUID ${SP_UUID}
	SP_NAME ${SP_NAME}
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_${SP_NAME}.dts.in
	DTS_MEM_REGIONS ${SP_UUID}_memory_regions.dtsi
	JSON_IN ${TS_ROOT}/environments/sp/sp_pkg.json.in
)

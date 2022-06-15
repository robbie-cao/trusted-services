#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the protected-storage deployment for generic
#  sp environment.
#
#  Builds the secure storage service provider for running in an SEL0 secure
#  partition hosted by any SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/sp/env.cmake)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
project(trusted-services LANGUAGES C ASM)
add_executable(protected-storage)
target_include_directories(protected-storage PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_NAME "protected-storage")
set(SP_UUID_CANON "751bf801-3dde-4768-a514-0f10aeed1790")
set(SP_UUID_LE "0x01f81b75 0x6847de3d 0x100f14a5 0x9017edae")
set(TRACE_PREFIX "PS" CACHE STRING "Trace prefix")
set(SP_STACK_SIZE "64 * 1024" CACHE STRING "Stack size")
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "Heap size")

add_components(TARGET "protected-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		environments/sp
)

include(../protected-storage.cmake REQUIRED)

target_compile_definitions(protected-storage PRIVATE
	ARM64=1
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(protected-storage PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-std=c99
	)

	# Options for GCC that control linking
	target_link_options(protected-storage PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(protected-storage PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
	)
endif()

compiler_generate_binary_output(TARGET protected-storage NAME "${SP_UUID_CANON}.bin" SP_BINARY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP_UUID_CANON}.bin DESTINATION ${TS_ENV}/bin)

include(${TS_ROOT}/tools/cmake/common/ExportMemoryRegionsToManifest.cmake REQUIRED)
export_memory_regions_to_manifest(TARGET protected-storage NAME "${SP_UUID_CANON}_memory_regions.dtsi" RES EXPORT_MEMORY_REGIONS_DTSI)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS protected-storage
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

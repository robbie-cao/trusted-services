#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the protected-storage deployment for opteesp
#
#  Builds the secure storage service provider for running in an SEL0 secure
#  partition hosted by OPTEE in the role of SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(protected-storage)
target_include_directories(protected-storage PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID_CANON "751bf801-3dde-4768-a514-0f10aeed1790")
set(SP_UUID_LE "0x01f81b75 0x6847de3d 0x100f14a5 0x9017edae")
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "SP heap size in bytes")
set(TRACE_PREFIX "PS" CACHE STRING "Trace prefix")

add_components(TARGET "protected-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		environments/opteesp
)

include(../protected-storage.cmake REQUIRED)

target_compile_definitions(protected-storage PRIVATE
	ARM64=1
)

target_include_directories(protected-storage PRIVATE
	${TS_ROOT}/deployments/protected-storage/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(protected-storage PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
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
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET protected-storage NAME "${SP_UUID_CANON}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS protected-storage
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake)
export_sp(
	SP_UUID_CANON ${SP_UUID_CANON}
	SP_UUID_LE ${SP_UUID_LE}
	SP_NAME "protected-storage"
	MK_IN ${TS_ROOT}/environments/opteesp/sp.mk.in
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_protected-storage.dts.in
	JSON_IN ${TS_ROOT}/environments/opteesp/sp_pkg.json.in
)

#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the sfs-demo deployment for opteesp
#
#  Used for building a demo sp that acts as a client of the secure storage
#  service, deployed in another sp.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(sfs-demo)
set(SP_UUID_CANON "01109cf8-e5ca-446f-9b55-f3cdc65110c8")
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "SP heap size in bytes")
set(TRACE_PREFIX "SFSDEMO" CACHE STRING "Trace prefix")

add_components(TARGET "sfs-demo"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		environments/opteesp
)

include(../sfs-demo.cmake REQUIRED)

target_compile_definitions(sfs-demo PRIVATE
	ARM64=1
)

target_include_directories(sfs-demo PRIVATE
	${TS_ROOT}/deployments/sfs-demo/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(sfs-demo PRIVATE
		-std=c99
	)

endif()

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS sfs-demo
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake)
export_sp(
	SP_UUID_CANON ${SP_UUID_CANON}
	SP_UUID_LE ${SP_UUID_LE}
	SP_NAME "sfs-demo"
	MK_IN ${TS_ROOT}/environments/opteesp/sp.mk.in
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_sfs-demo.dts.in
	JSON_IN ${TS_ROOT}/environments/opteesp/sp_pkg.json.in
)

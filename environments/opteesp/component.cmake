#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/optee_sp_header.c"
	"${CMAKE_CURRENT_LIST_DIR}/sp_entry.c"
	"${CMAKE_CURRENT_LIST_DIR}/sp_trace.c"
)

target_include_directories(${TGT}
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/include"
	)

# Default trace configuration, can be overwritten by setting the same variables
# in the deployment specific file before including this file.
set(TRACE_PREFIX "SP" CACHE STRING "Trace prefix")
set(TRACE_LEVEL "TRACE_LEVEL_ERROR" CACHE STRING "Trace level")

if (NOT DEFINED SP_HEAP_SIZE)
	message(FATAL_ERROR "SP_HEAP_SIZE is not defined")
endif()

target_compile_definitions(${TGT} PRIVATE
	TRACE_LEVEL=${TRACE_LEVEL}
	TRACE_PREFIX="${TRACE_PREFIX}"
	SP_HEAP_SIZE=${SP_HEAP_SIZE}
)

include(../../../external/newlib/newlib.cmake)

target_link_libraries(${TGT} PRIVATE
 	stdlib::c
)

target_link_options(${TGT} PRIVATE
	-Wl,--hash-style=sysv
	-Wl,--as-needed
	-Wl,--gc-sections
)

compiler_set_linker_script(TARGET ${TGT} FILE ${CMAKE_CURRENT_LIST_DIR}/sp.ld.S DEF ARM64=1)

#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/libsp_entry.c"
	"${CMAKE_CURRENT_LIST_DIR}/sp_trace.c"
	)

target_include_directories(${TGT}
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/include"
	)

if (NOT DEFINED TRACE_PREFIX)
	set(TRACE_PREFIX "SP" CACHE STRING "Trace prefix")
endif()

if (NOT DEFINED TRACE_LEVEL)
	set(TRACE_LEVEL "TRACE_LEVEL_ERROR" CACHE STRING "Trace level")
endif()

target_compile_definitions(${TGT} PRIVATE
	TRACE_LEVEL=${TRACE_LEVEL}
	TRACE_PREFIX="${TRACE_PREFIX}"
)

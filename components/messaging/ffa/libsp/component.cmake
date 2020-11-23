#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/aarch64/ffa_syscalls_a64.S"
	"${CMAKE_CURRENT_LIST_DIR}/ffa.c"
	"${CMAKE_CURRENT_LIST_DIR}/ffa_interrupt_handler.c"
	"${CMAKE_CURRENT_LIST_DIR}/ffa_memory_descriptors.c"
	"${CMAKE_CURRENT_LIST_DIR}/sp_rxtx.c"
	)

set_property(TARGET ${TGT} PROPERTY PUBLIC_HEADER
	${CMAKE_CURRENT_LIST_DIR}/include/ffa_api_defines.h
	${CMAKE_CURRENT_LIST_DIR}/include/ffa_api_types.h
	${CMAKE_CURRENT_LIST_DIR}/include/ffa_api.h
	${CMAKE_CURRENT_LIST_DIR}/include/ffa_internal_api.h
	${CMAKE_CURRENT_LIST_DIR}/include/ffa_memory_descriptors.h
	${CMAKE_CURRENT_LIST_DIR}/include/sp_api_defines.h
	${CMAKE_CURRENT_LIST_DIR}/include/sp_api_types.h
	${CMAKE_CURRENT_LIST_DIR}/include/sp_api.h
	${CMAKE_CURRENT_LIST_DIR}/include/sp_rxtx.h
	)


target_include_directories(${TGT}
	 PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
		"$<INSTALL_INTERFACE:include>"
	)

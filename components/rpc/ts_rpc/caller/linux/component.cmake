#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

include(${TS_ROOT}/external/LinuxFfaTeeDriver/LinuxFfaTeeDriver.cmake)

set_property(TARGET ${TGT} APPEND PROPERTY PUBLIC_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/ts_rpc_caller_linux.h"
	)

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ts_rpc_caller_linux.c"
	)

target_include_directories(${TGT} PRIVATE
	"${LINUX_FFA_TEE_DRIVER_INCLUDE_DIR}"
	)

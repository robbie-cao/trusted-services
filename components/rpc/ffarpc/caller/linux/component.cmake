#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if(NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

include(${TS_ROOT}/external/LinuxFfaTeeDriver/LinuxFfaTeeDriver.cmake)

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ffarpc_caller.c"
	"${CMAKE_CURRENT_LIST_DIR}/ffa_tee.c"
)

target_include_directories(${TGT} PRIVATE
	"${LINUX_FFA_TEE_DRIVER_INCLUDE_DIR}"
)

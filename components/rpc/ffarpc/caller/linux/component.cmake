#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if(NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

include(${TS_ROOT}/external/LinuxFFAUserShim/LinuxFFAUserShim.cmake)

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ffarpc_caller.c"
)

target_include_directories(${TGT} PRIVATE
	"${LINUX_FFA_USER_SHIM_INCLUDE_DIR}"
)

#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()


target_include_directories(${TGT}
	 PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/include"
	)

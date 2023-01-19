#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/fwu_app.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/metadata_reader.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/metadata_v1_reader.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/metadata_v2_reader.cpp"
	)

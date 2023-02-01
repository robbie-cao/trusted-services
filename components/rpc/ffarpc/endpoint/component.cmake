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
	"${CMAKE_CURRENT_LIST_DIR}/ffarpc_call_ep.c"
	)

if(NOT ((TS_ENV STREQUAL "linux-pc") OR (TS_ENV STREQUAL "arm-linux")))
	if(NOT DEFINED TS_RPC_UUID_CANON)
		message(FATAL_ERROR "Mandatory parameter TS_RPC_UUID_CANON is not defined.")
	endif()

	# Verify that SP is using the TS protocol.
	if (NOT "${SP_FFA_UUID_CANON}" STREQUAL "${TS_RPC_UUID_CANON}")
		message(FATAL_ERROR "The code is using TS RPC, but the SP_FFA_UUID_CANON is not matching the RPC UUID.")
	endif()
endif()

#-------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

set_property(TARGET ${TGT} APPEND PROPERTY PUBLIC_HEADER
	"${CMAKE_CURRENT_LIST_DIR}/rss_comms_messenger_api.h"
	)

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/rss_comms_messenger.c"
	"${CMAKE_CURRENT_LIST_DIR}/rss_comms_shm.c"
	"${CMAKE_CURRENT_LIST_DIR}/rss_comms_mhu.c"
	)

target_include_directories(${TGT}
	 PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}"
	)

#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Add source files for using mhu driver
if(PLAT_MHU_VERSION EQUAL 3)
	target_sources(${TGT}
		PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/mhu_v3_x.c"
		"${CMAKE_CURRENT_LIST_DIR}/mhu_wrapper_v3_x.c"
	)
else()
	target_sources(${TGT}
		PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/mhu_v2_x.c"
	)
endif()

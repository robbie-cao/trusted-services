#-------------------------------------------------------------------------------
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform definition for the 'fvp_base_revc-2xaem8a' virtual platform.
#-------------------------------------------------------------------------------

target_compile_definitions(${TGT} PRIVATE
	SMM_VARIABLE_INDEX_STORAGE_UID=0x787
	SMM_GATEWAY_MAX_UEFI_VARIABLES=80
)

get_property(_platform_driver_dependencies TARGET ${TGT}
	PROPERTY TS_PLATFORM_DRIVER_DEPENDENCIES
)

#-------------------------------------------------------------------------------
#  Map platform dependencies to suitable drivers for this platform
#
#-------------------------------------------------------------------------------
if ("mhu" IN_LIST _platform_driver_dependencies)
	include(${TS_ROOT}/platform/drivers/arm/mhu_driver/mhu_v2_x/driver.cmake)
endif()

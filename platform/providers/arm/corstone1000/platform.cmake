#-------------------------------------------------------------------------------
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform definition for the 'fvp_base_revc-2xaem8a' virtual platform.
#-------------------------------------------------------------------------------

set(SMM_GATEWAY_MAX_UEFI_VARIABLES 80 CACHE STRING "Maximum UEFI variable count")
set(SMM_RPC_CALLER_SESSION_SHARED_MEMORY_SIZE 4*4096 CACHE STRING "RPC caller buffer size in SMMGW")
set(SMM_SP_HEAP_SIZE 48*1024 CACHE STRING "SMM gateway SP heap size")

target_compile_definitions(${TGT} PRIVATE
	SMM_VARIABLE_INDEX_STORAGE_UID=0x787
	PLAT_RSS_COMMS_PAYLOAD_MAX_SIZE=0x2080
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

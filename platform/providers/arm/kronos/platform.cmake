#-------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform definition for the 'FVP_Kronos' virtual platform.
#-------------------------------------------------------------------------------

# include MHU driver
include(${TS_ROOT}/platform/drivers/arm/mhu_driver/component.cmake)

if (NOT DEFINED TGT)
	message(FATAL_ERROR "Mandatory parameter TGT is not defined.")
endif()

target_compile_definitions(${TGT} PRIVATE
	SMM_GATEWAY_NV_STORE_SN="sn:ffa:46bb39d1-b4d9-45b5-88ff-040027dab249:1"
	SMM_VARIABLE_INDEX_STORAGE_UID=0x787
	SMM_GATEWAY_MAX_UEFI_VARIABLES=100
)

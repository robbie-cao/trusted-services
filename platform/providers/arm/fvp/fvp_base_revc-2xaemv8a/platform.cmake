#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform definition for the 'fvp_base_revc-2xaem8a' virtual platform.
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

get_property(_platform_driver_dependencies TARGET ${TGT}
	PROPERTY TS_PLATFORM_DRIVER_DEPENDENCIES
)

#-------------------------------------------------------------------------------
#  Map platform dependencies to suitable drivers for this platform
#
#-------------------------------------------------------------------------------
if ("trng" IN_LIST _platform_driver_dependencies)
	include(${TS_ROOT}/platform/drivers/arm/juno_trng/driver.cmake)
endif()

if ("secure-nor-flash" IN_LIST _platform_driver_dependencies)
	include(${TS_ROOT}/platform/drivers/tf-a/drivers/cfi/v2m/v2m_flash.cmake)
endif()
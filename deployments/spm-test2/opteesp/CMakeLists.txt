#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")
include(../../deployment.cmake REQUIRED)
include(${TS_ROOT}/environments/opteesp/env.cmake)

project(trusted-services LANGUAGES C ASM)
add_executable(spm-test2)

set(SP_BIN_UUID_CANON "7817164c-c40c-4d1a-867a-9bb2278cf41a")
set(SP_UUID_CANON "7817164c-c40c-4d1a-867a-9bb2278cf41a")
set(SP_NUMBER 2)
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "SP heap size in bytes")
set(TRACE_PREFIX "SPM-TEST${SP_NUMBER}" CACHE STRING "Trace prefix")

#-------------------------------------------------------------------------------
#  Components that are spm-test specific to deployment in the opteesp
#  environment.
#-------------------------------------------------------------------------------
add_components(TARGET "spm-test${SP_NUMBER}"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/fdt"
		"components/common/trace"
		"components/common/utils"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"environments/opteesp"
)

include(${TS_ROOT}/components/service/spm_test/spm_test.cmake)

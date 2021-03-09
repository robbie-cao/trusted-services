#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if(NOT CROSS_COMPILE AND NOT DEFINED ENV{CROSS_COMPILE})
        set(CROSS_COMPILE "aarch64-linux-gnu-;aarch64-none-linux-gnu-" CACHE STRING "List of GCC prefix triplets to use.")
endif()

set(CMAKE_CROSSCOMPILING True)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_FLAGS_INIT "-fdiagnostics-show-option -gdwarf-2 -mstrict-align -O0 -DARM64=1")
set(CMAKE_CXX_FLAGS_INIT "-fdiagnostics-show-option -gdwarf-2 -mstrict-align -O0 -DARM64=1")

include($ENV{TS_ROOT}/tools/cmake/compiler/GCC.cmake REQUIRED)

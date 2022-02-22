#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#GNUARM v8 and v9 compilers use a different triplet.
if(NOT CROSS_COMPILE AND NOT DEFINED ENV{CROSS_COMPILE})
	set(CROSS_COMPILE "aarch64-elf-;aarch64-none-elf-;aarch64-linux-gnu-;aarch64-none-linux-gnu-" CACHE STRING "List of GCC prefix triplets to use.")
endif()

set(CMAKE_CROSSCOMPILING True)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

include($ENV{TS_ROOT}/tools/cmake/compiler/GCC.cmake REQUIRED)
include($ENV{TS_ROOT}/tools/cmake/compiler/config_iface.cmake REQUIRED)

# Set mandatory compiler and linker flags for this environment:
#   - Compile position independent code
string(APPEND CMAKE_C_FLAGS_INIT " -fpic")

# Set compiler warning level for the root build context. External components
# are responsible for setting their own warning level.
if(DEFINED TS_ROOT)
    string(APPEND CMAKE_C_FLAGS_INIT " -Wall")
endif()

#   - Link position independent executable
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -pie")

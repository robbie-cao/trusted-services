#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
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
#   - Disable startup files and default libraries.
#   - Link position independent executable
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -nostartfiles -nodefaultlibs -pie")

#   -link libgcc with full PATH to work around disabled linker search paths.
gcc_get_lib_location("libgcc.a" _TMP_VAR)
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${_TMP_VAR} ")
unset(_TMP_VAR)

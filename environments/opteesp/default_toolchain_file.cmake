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

#set(CMAKE_C_FLAGS_INIT --specs=nosys.specs)
#set(CMAKE_CXX_FLAGS_INIT --specs=nosys.specs)
#set(CMAKE_EXE_LINKER_FLAGS_INIT --specs=nosys.specs)

include($ENV{TS_ROOT}/tools/cmake/compiler/GCC.cmake REQUIRED)

# Set mandatory compiler and linker flags for this environment:
#   - This environment uses a libc implementation from SPDEV-KIT. Disable standard
#     include search paths, startup files and default libraries.
string(APPEND CMAKE_C_FLAGS_INIT " -nostartfiles -nodefaultlibs -nostdinc -I ${CMAKE_CURRENT_LIST_DIR}/include")
#   - Compile position independent code
string(APPEND CMAKE_C_FLAGS_INIT " -fpic")
#   -set entry point
#   -disable link time optimization
#   -link position independent executable
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -e __sp_entry -fno-lto -pie")

#   -link libgcc with full PATH to work around disabled linker search paths.
gcc_get_lib_location("libgcc.a" _TMP_VAR)
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${_TMP_VAR} ")
unset(_TMP_VAR)

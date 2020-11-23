#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#GNUARM v8 and v9 compilers use a different triplet.
if(NOT DEFINED ENV{CROSS_COMPILE})
	set(CROSS_COMPILE "aarch64-elf-;aarch64-none-elf-;aarch64-linux-gnu-" CACHE STRING "List of GCC prefix triplets to use.")
endif()

set(CMAKE_CROSSCOMPILING True)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

#set(CMAKE_C_FLAGS_INIT --specs=nosys.specs)
#set(CMAKE_CXX_FLAGS_INIT --specs=nosys.specs)
#set(CMAKE_EXE_LINKER_FLAGS_INIT --specs=nosys.specs)

include($ENV{TS_ROOT}/tools/cmake/compiler/GCC.cmake REQUIRED)

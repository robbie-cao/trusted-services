#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
FindLibClang
------------

CMake module for finding the LibClang library.

Control flow
^^^^^^^^^^^^

1. Running ``llvm-config`` if exists
2. Searching for library at common places
3. Searching in Windows registry if available


Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LibClang``
  The Clang library


Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LibClang_FOUND``
  True if the system has the Clang library.
``LibClang_LIBRARY_DIRS``
  Libraries needed to link to Clang.

#]=======================================================================]


# 1. Use llvm-config
find_program(_LLVM_CONFIG_COMMAND "llvm-config")

if (_LLVM_CONFIG_COMMAND)
	message(STATUS "Setting LibClang_LIBRARY_DIRS using ${_LLVM_CONFIG_COMMAND}")

	execute_process(
		COMMAND ${_LLVM_CONFIG_COMMAND} --libdir
		OUTPUT_VARIABLE _LLVM_CONFIG_OUTPUT
	)

	# Stripping newline
	string(STRIP ${_LLVM_CONFIG_OUTPUT} LibClang_LIBRARY_DIRS)
endif()

# 2. Try to find as library
if (NOT LibClang_LIBRARY_DIRS)
	message(STATUS "Setting LibClang_LIBRARY_DIRS based on common directories list")

	set(LIBCLANG_COMMON_PATHS
		/usr/lib/llvm-9/lib
		/usr/lib/llvm-8/lib
		/usr/lib/llvm-7/lib
		/usr/lib/llvm-6.0/lib)

	if (WIN32)
		set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")

		get_filename_component(LLVM_PATH_FROM_REGISTRY [HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\LLVM\\LLVM] ABSOLUTE)
		list(APPEND LIBCLANG_COMMON_PATHS "${LLVM_PATH_FROM_REGISTRY}/bin")
	endif()

	find_library(_LIBCLANG_PATH
		NAMES clang
		HINTS ${LIBCLANG_COMMON_PATHS}
	)

	if (_LIBCLANG_PATH)
		get_filename_component(LibClang_LIBRARY_DIRS ${_LIBCLANG_PATH} DIRECTORY)
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibClang
	"Please install llvm-config or set LibClang path manually"
	LibClang_LIBRARY_DIRS
)

#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Determine the number of processes to run while running parallel builds.
# Pass -DPROCESSOR_COUNT=<n> to cmake to override.
if(NOT DEFINED PROCESSOR_COUNT)
	include(ProcessorCount)
	ProcessorCount(PROCESSOR_COUNT)
	set(PROCESSOR_COUNT ${PROCESSOR_COUNT} CACHE STRING "Number of cores to use for parallel builds.")
endif()

if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

# Compile TS specific newlib porting files.
target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/newlib_init.c"
	"${CMAKE_CURRENT_LIST_DIR}/newlib_sp_assert.c"
	"${CMAKE_CURRENT_LIST_DIR}/newlib_sp_heap.c"
)

set(NEWLIB_URL "https://sourceware.org/git/newlib-cygwin.git" CACHE STRING "newlib repository URL")
set(NEWLIB_REFSPEC "newlib-4.1.0" CACHE STRING "newlib git refspec")
set(NEWLIB_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/newlib_install" CACHE PATH "newlib installation directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching newlib
FetchContent_Declare(
	newlib
	GIT_REPOSITORY ${NEWLIB_URL}
	GIT_TAG ${NEWLIB_REFSPEC}
	GIT_SHALLOW TRUE
	PATCH_COMMAND git stash
		COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/0001-Allow-aarch64-linux-gcc-to-compile-bare-metal-lib.patch
		COMMAND git reset HEAD~1
)

# FetchContent_GetProperties exports newlib_SOURCE_DIR and newlib_BINARY_DIR variables
FetchContent_GetProperties(newlib)
if(NOT newlib_POPULATED)
	message(STATUS "Fetching newlib")
	FetchContent_Populate(newlib)
endif()

# Extracting compiler prefix without the trailing hyphen from the C compiler name
get_filename_component(COMPILER_PATH ${CMAKE_C_COMPILER} DIRECTORY)
get_filename_component(COMPILER_NAME ${CMAKE_C_COMPILER} NAME)
string(REGEX REPLACE "([^-]+-[^-]+-[^-]+).*" "\\1" COMPILER_PREFIX ${COMPILER_NAME})

# Newlib configure step
# CC env var must be unset otherwise configure will assume the cross compiler is the host compiler
# The configure options are set to minimize code size and memory usage.
execute_process(COMMAND
	${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH} ./configure
		--target=${COMPILER_PREFIX}
		--prefix=${NEWLIB_INSTALL_PATH}
		--enable-newlib-nano-formatted-io
		--enable-newlib-nano-malloc
		--enable-lite-exit
		--enable-newlib-reent-small
		--enable-newlib-global-atexit
		--disable-multilib
		CFLAGS_FOR_TARGET=-fpic
		LDFLAGS_FOR_TARGET=-fpie
	WORKING_DIRECTORY
		${newlib_SOURCE_DIR}
	RESULT_VARIABLE _newlib_error
)

if (_newlib_error)
	message(FATAL_ERROR "Configuration step of newlib failed with ${_newlib_error}.")
endif()

# Newlib build step
execute_process(COMMAND
	${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH} make -j${PROCESSOR_COUNT}
	WORKING_DIRECTORY
		${newlib_SOURCE_DIR}
	RESULT_VARIABLE _newlib_error
)

if (_newlib_error)
	message(FATAL_ERROR "Build step of newlib failed with ${_newlib_error}.")
endif()

# Newlib install step
execute_process(COMMAND
	${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH} make install
	WORKING_DIRECTORY
		${newlib_SOURCE_DIR}
	RESULT_VARIABLE _newlib_error
)

if (_newlib_error)
	message(FATAL_ERROR "Install step of newlib failed with ${_newlib_error}.")
endif()

# libc
add_library(c STATIC IMPORTED)
set_property(TARGET c PROPERTY IMPORTED_LOCATION "${NEWLIB_INSTALL_PATH}/${COMPILER_PREFIX}/lib/libc.a")
set_property(TARGET c PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_PATH}/${COMPILER_PREFIX}/include")
target_compile_options(c INTERFACE -nostdinc)
target_link_options(c INTERFACE -nostartfiles -nodefaultlibs)

# libnosys
add_library(nosys STATIC IMPORTED)
set_property(TARGET nosys PROPERTY IMPORTED_LOCATION "${NEWLIB_INSTALL_PATH}/${COMPILER_PREFIX}/lib/libnosys.a")
set_property(TARGET nosys PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_PATH}/${COMPILER_PREFIX}/include")
target_compile_options(nosys INTERFACE -nostdinc)
target_link_options(nosys INTERFACE -nostartfiles -nodefaultlibs)
target_link_libraries(c INTERFACE nosys)

# Make standard library available in the build system.
add_library(stdlib::c ALIAS c)

# -noxxx options above require explicitly naming GCC specific library on the
# linker command line.
include(${TS_ROOT}/tools/cmake/compiler/GCC.cmake)
gcc_get_lib_location(LIBRARY_NAME "libgcc.a" RES _TMP_VAR)
link_libraries(${_TMP_VAR})
# Moreover the GCC specific header file include directory is also required.
# There is no way to stop cmake from filtering out built in compiler include paths
# from compiler command line (see https://gitlab.kitware.com/cmake/cmake/-/issues/19227)
# As a workaround copy headers to build directory and set include path to the new
# location.
get_filename_component(_TMP_VAR "${_TMP_VAR}" DIRECTORY)
file(COPY "${_TMP_VAR}/include" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/gcc_include)
file(COPY "${_TMP_VAR}/include-fixed" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/gcc-include)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gcc_include/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gcc-include/include-fixed")
unset(_TMP_VAR)

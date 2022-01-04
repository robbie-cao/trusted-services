#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Add newlib specific porting files to the project.
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

# Compile TS specific newlib porting files.
target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/newlib_init.c"
	"${CMAKE_CURRENT_LIST_DIR}/newlib_sp_assert.c"
	"${CMAKE_CURRENT_LIST_DIR}/newlib_sp_heap.c"
)

# Fetch newlib from external repository
set(NEWLIB_URL "https://sourceware.org/git/newlib-cygwin.git"
		CACHE STRING "newlib repository URL")
set(NEWLIB_REFSPEC "newlib-4.1.0"
		CACHE STRING "newlib git refspec")
set(NEWLIB_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/newlib-src"
		CACHE PATH "newlib source-code location")
set(NEWLIB_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/newlib_install"
		CACHE PATH "newlib installation directory")

# Extracting compiler prefix without the trailing hyphen from the C compiler name
get_filename_component(COMPILER_PATH ${CMAKE_C_COMPILER} DIRECTORY)
get_filename_component(COMPILER_NAME ${CMAKE_C_COMPILER} NAME)
string(REGEX REPLACE "(.*)-[^-]+$" "\\1" COMPILER_PREFIX ${COMPILER_NAME})

find_library(NEWLIB_LIBC_PATH
				NAMES libc.a c.a libc.lib c.lib
				PATHS ${NEWLIB_INSTALL_DIR}
				PATH_SUFFIXES "${COMPILER_PREFIX}/lib"
				DOC "Location of newlib::libc library."
				NO_DEFAULT_PATH
)
set(NEWLIB_LIBC_PATH ${NEWLIB_LIBC_PATH})
unset(NEWLIB_LIBC_PATH CACHE)

find_library(NEWLIB_LIBNOSYS_PATH
				NAMES libnosys.a nosys.a nosys.lib nosys.lib
				PATHS ${NEWLIB_INSTALL_DIR}
				PATH_SUFFIXES "${COMPILER_PREFIX}/lib"
				DOC "Location of newlib::libnosys library."
				NO_DEFAULT_PATH
)
set(NEWLIB_LIBNOSYS_PATH ${NEWLIB_LIBC_PATH})
unset(NEWLIB_LIBNOSYS_PATH CACHE)

if (NOT NEWLIB_LIBC_PATH)
	# Determine the number of processes to run while running parallel builds.
	# Pass -DPROCESSOR_COUNT=<n> to cmake to override.
	if(NOT DEFINED PROCESSOR_COUNT)
		include(ProcessorCount)
		ProcessorCount(PROCESSOR_COUNT)
		set(PROCESSOR_COUNT ${PROCESSOR_COUNT}
				CACHE STRING "Number of cores to use for parallel builds.")
	endif()

	# See if the source is available locally
	find_file(NEWLIB_HEADER_FILE
		NAMES newlib.h
		PATHS ${NEWLIB_SOURCE_DIR}
		PATH_SUFFIXES "newlib/libc/include"
		NO_DEFAULT_PATH
	)
	set(NEWLIB_HEADER_FILE ${NEWLIB_HEADER_FILE})
	unset(NEWLIB_HEADER_FILE CACHE)

	# Source not found, fetch it.
	if (NOT NEWLIB_HEADER_FILE)
		include(FetchContent)

		# Checking git
		find_program(GIT_COMMAND "git")
		if (NOT GIT_COMMAND)
			message(FATAL_ERROR "Please install git")
		endif()

		# Fetching newlib
		FetchContent_Declare(
			newlib
			SOURCE_DIR ${NEWLIB_SOURCE_DIR}
			GIT_REPOSITORY ${NEWLIB_URL}
			GIT_TAG ${NEWLIB_REFSPEC}
			GIT_SHALLOW TRUE
			PATCH_COMMAND git stash
				COMMAND ${GIT_COMMAND} am ${CMAKE_CURRENT_LIST_DIR}/0001-Allow-aarch64-linux-gcc-to-compile-bare-metal-lib.patch
				COMMAND ${GIT_COMMAND} reset HEAD~1
		)

		# FetchContent_GetProperties exports newlib_SOURCE_DIR and newlib_BINARY_DIR variables
		FetchContent_GetProperties(newlib)
		# FetchContent_Populate will fail if the source directory is removed since it will try to
		# do an "update" and not a "populate" action. As a workaround, remove the subbuild directory.
		# Note: this fix assumes, the default subbuild location is used.
		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/_deps/newlib-subbuild")

		if(NOT newlib_POPULATED)
			message(STATUS "Fetching newlib")
			FetchContent_Populate(newlib)
		endif()
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${NEWLIB_SOURCE_DIR})
	endif()

	# Newlib configure step
	# CC env var must be unset otherwise configure will assume the cross compiler is the host
	# compiler.
	# The configure options are set to minimize code size and memory usage.
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH} ./configure
			--target=${COMPILER_PREFIX}
			--prefix=${NEWLIB_INSTALL_DIR}
			--enable-newlib-nano-formatted-io
			--enable-newlib-nano-malloc
			--enable-lite-exit
			--enable-newlib-reent-small
			--enable-newlib-global-atexit
			--disable-multilib
			CFLAGS_FOR_TARGET=-fpic
			LDFLAGS_FOR_TARGET=-fpie
		WORKING_DIRECTORY
			${NEWLIB_SOURCE_DIR}
		RESULT_VARIABLE _newlib_error
	)

	if (_newlib_error)
		message(FATAL_ERROR "Configuration step of newlib failed with ${_newlib_error}.")
	endif()

	# Newlib build step
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH}
			make -j${PROCESSOR_COUNT}
		WORKING_DIRECTORY
			${NEWLIB_SOURCE_DIR}
		RESULT_VARIABLE _newlib_error
	)

	if (_newlib_error)
		message(FATAL_ERROR "Build step of newlib failed with ${_newlib_error}.")
	endif()

	# Newlib install step
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH} make install
		WORKING_DIRECTORY
			${NEWLIB_SOURCE_DIR}
		RESULT_VARIABLE _newlib_error
	)

	if (_newlib_error)
		message(FATAL_ERROR "Install step of newlib failed with ${_newlib_error}.")
	endif()

	set(NEWLIB_LIBC_PATH "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/lib/libc.a")
	set(NEWLIB_LIBNOSYS_PATH "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/lib/libnosys.a")
endif()

set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
	APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${NEWLIB_LIBC_PATH})

# libc
add_library(c STATIC IMPORTED)
set_property(TARGET c PROPERTY IMPORTED_LOCATION "${NEWLIB_LIBC_PATH}")
set_property(TARGET c PROPERTY
		INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/include")
target_compile_options(c INTERFACE -nostdinc)
target_link_options(c INTERFACE -nostartfiles -nodefaultlibs)

# libnosys
add_library(nosys STATIC IMPORTED)
set_property(TARGET nosys PROPERTY IMPORTED_LOCATION "${NEWLIB_LIBNOSYS_PATH}")
set_property(TARGET nosys PROPERTY
		INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/include")
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

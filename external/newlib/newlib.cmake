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

# Adding libc interface
add_components(TARGET ${TGT}
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/common/libc
)

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
set(NEWLIB_LIBNOSYS_PATH ${NEWLIB_LIBNOSYS_PATH})
unset(NEWLIB_LIBNOSYS_PATH CACHE)

# -noxxx options require explicitly specifying GCC specific library on the
# linker command line.
# Use LIBGCC_PATH to manually override the search process below.
if (NOT DEFINED LIBGCC_PATH)
	include(${TS_ROOT}/tools/cmake/compiler/GCC.cmake)
	gcc_get_lib_location(LIBRARY_NAME "libgcc.a" RES _TMP_VAR)

	if (NOT _TMP_VAR)
		message(FATAL_ERROR "Location of libgcc.a can not be determined. Please set LIBGCC_PATH on the command line.")
	endif()
	set(LIBGCC_PATH ${_TMP_VAR} CACHE PATH "location of libgcc.a")
	unset(_TMP_VAR)
endif()

if(NOT EXISTS "${LIBGCC_PATH}" OR IS_DIRECTORY "${LIBGCC_PATH}")
	message(FATAL_ERROR "LIBGCC_PATH \"${LIBGCC_PATH}\" must be the full path of a library file."
						" Either set LIBGCC_PATH on the command line, or fix the value if already set.")
endif()
message(STATUS "libgcc.a is used from ${LIBGCC_PATH}")

# Moreover the GCC specific header file include directory is also required.
# Specify LIBGCC_INCLUDE_DIRS in the command line to manually override the libgcc relative location below.
if(NOT DEFINED LIBGCC_INCLUDE_DIRS)
	get_filename_component(_TMP_VAR "${LIBGCC_PATH}" DIRECTORY)
	set(LIBGCC_INCLUDE_DIRS
		"${_TMP_VAR}/include"
		"${_TMP_VAR}/include-fixed" CACHE STRING "GCC specific include PATHs")
	unset(_TMP_VAR)
endif()

# There is no way to stop cmake from filtering out built in compiler include paths
# from compiler command line (see https://gitlab.kitware.com/cmake/cmake/-/issues/19227)
# As a workaround copy headers to build directory and set include path to the new
# location.
foreach(_dir IN LISTS LIBGCC_INCLUDE_DIRS)
	if(NOT IS_DIRECTORY "${_dir}")
		message(FATAL_ERROR "GCC specific include PATH \"${_dir}\" does not exist. Try setting LIBGCC_INCLUDE_DIRS"
							" on the command line.")
	endif()
	file(COPY "${_dir}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/gcc-include")
	get_filename_component(_TMP_VAR "${_dir}" NAME)
	list(APPEND _gcc_include_dirs "${CMAKE_CURRENT_BINARY_DIR}/gcc-include/${_TMP_VAR}")
	message(STATUS "Using compiler specific include path \"${_dir}\" mirrored to"
					"  \"${CMAKE_CURRENT_BINARY_DIR}/gcc-include/${_TMP_VAR}\".")
endforeach()
unset(_TMP_VAR)

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
			GIT_SHALLOW FALSE
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

	string(REPLACE ";" " -isystem " CFLAGS_FOR_TARGET "${_gcc_include_dirs}")
	set(CFLAGS_FOR_TARGET "-isystem ${CFLAGS_FOR_TARGET} -fpic")

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
			CFLAGS_FOR_TARGET=${CFLAGS_FOR_TARGET}
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
link_libraries(c INTERFACE ${LIBGCC_PATH})
set_property(TARGET c PROPERTY IMPORTED_LOCATION "${NEWLIB_LIBC_PATH}")
set_property(TARGET c PROPERTY
		INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/include")
target_compile_options(c INTERFACE -nostdinc)
target_link_options(c INTERFACE -nostartfiles -nodefaultlibs)
target_include_directories(c SYSTEM INTERFACE ${_gcc_include_dirs})

# Make standard library available in the build system.
add_library(stdlib::c ALIAS c)

# libnosys
add_library(nosys STATIC IMPORTED)
set_property(TARGET nosys PROPERTY IMPORTED_LOCATION "${NEWLIB_LIBNOSYS_PATH}")
set_property(TARGET nosys PROPERTY
		INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/include")
target_compile_options(nosys INTERFACE -nostdinc)
target_link_options(nosys INTERFACE -nostartfiles -nodefaultlibs)
target_link_libraries(c INTERFACE nosys)

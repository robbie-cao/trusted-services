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
# Supported build types: "Release" "Debug" "RelWithDebInfo" "Off"
# If set to "Off" -DNEWLIB_CFLAGS_TARGET can be used to set compiler
# switches from the command line.
set(NEWLIB_BUILD_TYPE "Release"	CACHE STRING "newlib build type")

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

# libc - get compiler specific configuration from GCC
add_library(c STATIC IMPORTED)
# We need "freestandig" mode. Ask the compile to do the needed configurations.
include(${TS_ROOT}/tools/cmake/compiler/GCC.cmake)
compiler_set_freestanding(TARGET c)

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

		# List patch files.
		file(GLOB _patch_files LIST_DIRECTORIES false "${CMAKE_CURRENT_LIST_DIR}/[0-9]*-[!0-9]*.patch")
		# Sort items in natural order to ensure patches are amended in the right order.
		list(SORT _patch_files COMPARE NATURAL)
		# Convert the list to a string of concatenated quoted list items.
		string(REPLACE ";" "\" \"" _patch_files "${_patch_files}")
		set(_patch_files "\"${_patch_files}\"")
		# Create a shell script patching newlib with the files listed above
		string(APPEND _patch_script "#!/bin/sh\n"
			" ${GIT_COMMAND} stash\n"
			" ${GIT_COMMAND} branch ts-bf-am\n"
			" ${GIT_COMMAND} am ${_patch_files}\n"
			" ${GIT_COMMAND} reset ts-bf-am\n"
			" ${GIT_COMMAND} branch -D ts-bf-am\n"
		)
		file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/patch-newlib "${_patch_script}")

		# Fetching newlib
		FetchContent_Declare(
			newlib
			SOURCE_DIR ${NEWLIB_SOURCE_DIR}
			GIT_REPOSITORY ${NEWLIB_URL}
			GIT_TAG ${NEWLIB_REFSPEC}
			GIT_SHALLOW FALSE
			PATCH_COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/patch-newlib
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

	# Get NEWLIB_EXTRA_PARAMS value from environment
	set(NEWLIB_EXTRA_PARAMS $ENV{NEWLIB_EXTRA_PARAMS} CACHE STRING "")

	# Split a newlib extra build parameter into a list of parameters
	set(_extra_params ${NEWLIB_EXTRA_PARAMS})
	separate_arguments(_extra_params)

	# Transfer libgcc specific settings to newlib, and set position independent compilation
	string(REPLACE ";" " -I" _more_cflags_target "${LIBGCC_INCLUDE_DIRS}" )
	set(_more_cflags_target "-fpic -I${_more_cflags_target}")

	string(TOUPPER ${NEWLIB_BUILD_TYPE} UC_NEWLIB_BUILD_TYPE)
	if ("${UC_NEWLIB_BUILD_TYPE}" STREQUAL "DEBUG")
		set(_more_cflags_target "${_more_cflags_target} -g -O0")
	elseif ("${UC_NEWLIB_BUILD_TYPE}" STREQUAL "RELEASE")
		set(_more_cflags_target "${_more_cflags_target} -O2")
	elseif ("${UC_NEWLIB_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")
		set(_more_cflags_target "${_more_cflags_target} -g -O2")
	elseif (NOT "${UC_NEWLIB_BUILD_TYPE}" STREQUAL "OFF")
		message(FATAL_ERROR "unsupported build type to newlib.")
	endif()

	# Get external extra flags for target from environment.
	set(NEWLIB_CFLAGS_TARGET $ENV{NEWLIB_CFLAGS_TARGET} CACHE STRING "")

	# Merge our CFLAGS with external CFLAGS
	if (NOT "${NEWLIB_CFLAGS_TARGET}" STREQUAL "")
		# Add a space separator if external value is not empty
		string(APPEND NEWLIB_CFLAGS_TARGET " ")
	endif()
	# Concatenate, and override CACHE value
	set(NEWLIB_CFLAGS_TARGET "${NEWLIB_CFLAGS_TARGET}${_more_cflags_target}" CACHE STRING "" FORCE)

	# Get extra external CFLAGS for host from environment
	set(NEWLIB_CFLAGS $ENV{NEWLIB_CFLAGS} CACHE STRING "")

	# Newlib is keeping build artifacts in the source directory. If the source is pre-fetched,
	# intermediate files of previoud build migth be still present.
	# Run distclean to avoid build errors due to reconfiguration.
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH}
			make -j${PROCESSOR_COUNT} distclean
		WORKING_DIRECTORY
			${NEWLIB_SOURCE_DIR}
		RESULT_VARIABLE _newlib_error
	)
	#ignore error as distclean-host is failing.
	#if (_newlib_error)
	#	message(FATAL_ERROR "\"distclean\" step of newlib failed with ${_newlib_error}.")
	#endif()

	# Newlib configure step
	# CC env var must be unset otherwise configure will assume the cross compiler is the host
	# compiler.
	# The configure options are set to minimize code size and memory usage.
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env --unset=CC PATH=${COMPILER_PATH}:$ENV{PATH} ./configure
			--target=${COMPILER_PREFIX}
			--host=${COMPILER_PREFIX}
			--prefix=${NEWLIB_INSTALL_DIR}
			--enable-newlib-nano-formatted-io
			--enable-newlib-nano-malloc
			--enable-lite-exit
			--enable-newlib-reent-small
			--enable-newlib-global-atexit
			--disable-multilib
			${_extra_params}
			CFLAGS_FOR_TARGET=${NEWLIB_CFLAGS_TARGET}
			CFLAGS=${NEWLIB_CFLAGS}
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

# libc - continue configuration
set_property(TARGET c PROPERTY IMPORTED_LOCATION "${NEWLIB_LIBC_PATH}")
target_compile_definitions(c INTERFACE ENABLE_CDEFSH_FIX)
set_property(TARGET c PROPERTY
		INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/include")

# Make standard library available in the build system.
add_library(stdlib::c ALIAS c)

# libnosys
add_library(nosys STATIC IMPORTED)
set_property(TARGET nosys PROPERTY IMPORTED_LOCATION "${NEWLIB_LIBNOSYS_PATH}")
set_property(TARGET nosys PROPERTY
		INTERFACE_INCLUDE_DIRECTORIES "${NEWLIB_INSTALL_DIR}/${COMPILER_PREFIX}/include")
compiler_set_freestanding(TARGET nosys)
target_link_libraries(c INTERFACE nosys)

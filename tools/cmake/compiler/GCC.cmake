#-------------------------------------------------------------------------------
# Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
Compiler abstraction for GCC
----------------------------

.. cmake:variable:: CROSS_COMPILE

	A ';' separated GCC prefix triplets to use when searching for the cross-compiler.
	(i.e. ``aarch64-none-elf;aarch64-elf``).
	The variable can be set on the command line with ``-DCROSS_COMPILE=<value>`` or in the
	environment. If both is specified, command line takes precedence.

#]===]

include_guard(DIRECTORY)

if(NOT CROSS_COMPILE AND NOT DEFINED ENV{CROSS_COMPILE})
	message(FATAL_ERROR "'CROSS_COMPILE' is not defined. Set it to the gcc pferix triplet, ie. cmake <..>-DCROSS_COMPILE=aarch64-elf-")
endif()

set(CROSS_COMPILE $ENV{CROSS_COMPILE} CACHE STRING "Prefix of the cross-compiler commands")

#Generate a list of tool names to look for. Store the result in CMAKE_<lang>_COMPILER.
function(gcc_find_tool NAME LANG)
	string(REGEX REPLACE "([^;]+);" "\\1${NAME};\\1${NAME}.exe;" _gcc_names "${CROSS_COMPILE};")
	find_program(_cross_compile_gcc NAMES ${_gcc_names} REQUIRED)
	if (NOT _cross_compile_gcc)
		string(REPLACE ";" " " _msg "${_gcc_names}")
		message(FATAL_ERROR "Failed to find ${NAME} with the names: ${_msg}")
	endif()
	set(CMAKE_${LANG}_COMPILER ${_cross_compile_gcc} CACHE STRING "${LANG} compiler executable.")
endfunction()

gcc_find_tool(gcc C)
gcc_find_tool(g++ CXX)

#Official solution to disable compiler checks
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#By default when INTERFACE_INCUDES of libraryes linked to an exe are treated
#as system includes. gcc-arm-8.2-2019.01-i686-mingw32-aarch64-elf (gcc 8.2.1) will
#set C linkage o these files, which will result in compilation errors for C++ projects.
#This setting fixes that.
set(CMAKE_NO_SYSTEM_FROM_IMPORTED True)

#[===[.rst:
.. cmake:command:: compiler_preprocess_file

  .. code-block:: cmake

    compiler_preprocess_file(SRC file.c DST file_pp.c)
    compiler_preprocess_file(SRC file.c DST file_pp.c
                             DEFINES USE_LIB INCLUDES include/lib)

  Run the preprocessor on a file and save the output to another file. Optionally
  provide defines and include paths to the preprocessor.

  Inputs:

  ``SRC``
    Name of the source file to preprocess.

  ``DST``
    Where to write the preprocessed output.

  ``DEFINES`` (multi, optional)
    Definitions for the preprocessor.

  ``INCLUDES`` (multi, optional)
    Include paths for the preprocessor.

#]===]
function(compiler_preprocess_file)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS SRC DST)
	set(_MULTI_VALUE_ARGS DEFINES INCLUDES)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	check_args(compiler_preprocess_file SRC DST)

	set(_flags "")
	if(_MY_PARAMS_DEFINES)
		list(TRANSFORM _MY_PARAMS_DEFINES PREPEND -D)
		list(APPEND _flags ${_MY_PARAMS_DEFINES})
	endif()
	if(_MY_PARAMS_INCLUDES)
		list(TRANSFORM _MY_PARAMS_INCLUDES PREPEND -I)
		list(APPEND _flags ${_MY_PARAMS_INCLUDES})
	endif()

	add_custom_command(
		DEPENDS ${_MY_PARAMS_SRC} OUTPUT ${_MY_PARAMS_DST}
		COMMAND ${CMAKE_C_COMPILER} -E -P -x assembler-with-cpp ${_flags}
				${_MY_PARAMS_SRC} -o ${_MY_PARAMS_DST}
	)
endfunction()

#[===[.rst:
.. cmake:command:: compiler_set_linker_script

  .. code-block:: cmake

    compiler_set_linker_script(TARGET foo FILE foo.ld.S)
    compiler_set_linker_script(TARGET foo FILE foo.ld.S DEF USE_LIB INC include/lib)

  Set linker script for a target. The function adds an LDFLAG using the
  toolchain specific syntax to the TARGET_linker_script group, which is applied
  onto the target by the caller function. FILE will be preprocessed, optionally
  defines and/or includes can be provided using DEF/INC arguments.

  Inputs:

  ``TARGET``
    Name of the target.

  ``FILE``
    Linker script file for the target.

  ``DEF`` (multi, optional)
    Defines for the linker script preprocessor.

  ``INC`` (multi, optional)
    Include paths for the linker script preprocessor.

#]===]
function(compiler_set_linker_script)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS TARGET FILE)
	set(_MULTI_VALUE_ARGS DEF INC)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	check_args(compiler_set_linker_script TARGET FILE)

	get_filename_component(_src "${_MY_PARAMS_FILE}" ABSOLUTE)
	get_filename_component(_src_ext "${_MY_PARAMS_FILE}" EXT)
	set(_dst "${CMAKE_BINARY_DIR}/${_MY_PARAMS_TARGET}.ld")

	if(NOT ("${_src_ext}" STREQUAL ".ld" OR "${_src_ext}" STREQUAL ".ld.S"))
		message(WARNING "compiler_set_linker_script(): extension mismatch '${_src}'")
	endif()

	compiler_preprocess_file(
		SRC ${_src}
		DST ${_dst}
		DEFINES ${_MY_PARAMS_DEF} __LINKER__
		INCLUDES ${_MY_PARAMS_INC}
	)

	add_custom_target("${_MY_PARAMS_TARGET}_ld" DEPENDS "${_dst}")
	add_dependencies("${_MY_PARAMS_TARGET}" "${_MY_PARAMS_TARGET}_ld")

	target_link_options(${_MY_PARAMS_TARGET} PRIVATE "-Wl,--script=${_dst}")
	set_target_properties(${_MY_PARAMS_TARGET} PROPERTIES LINK_DEPENDS "${_dst}")
endfunction()

#[===[.rst:
.. cmake:command:: compiler_generate_binary_output

  .. code-block:: cmake

    compiler_generate_binary_output(TARGET <name> RES <var>)

  Generate binary output for the target. The function converts the output
  executable into bin file using toolchain specific syntax.

  Inputs:

  ``TARGET``
    Name of the target.

  Outputs:

  ``RES``
    Full patch to output file.

#]===]
function(compiler_generate_binary_output)
	set(options)
	set(oneValueArgs TARGET)
	set(multiValueArgs)
	cmake_parse_arguments(MY "${options}" "${oneValueArgs}"
						"${multiValueArgs}" ${ARGN} )
	add_custom_command(
		TARGET ${MY_TARGET} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O binary
				$<TARGET_FILE:${MY_TARGET}>
				$<TARGET_FILE_DIR:${MY_TARGET}>/${MY_TARGET}.bin)
	if (MY_RES)
		set(${MY_RES} $<TARGET_FILE_DIR:${MY_TARGET}>/${MY_TARGET}.bin PARENT_SCOPE)
	endif()

endfunction()

#[===[.rst:
.. cmake:command:: compiler_generate_stripped_elf

  .. code-block:: cmake

    compiler_generate_stripped_elf(TARGET foo NAME foo.stripped.elf RES var)

  Strip all symbols that are not needed for relocation processing and return the location
  of the result.

  Inputs:

  ``TARGET``
    Name of the target.

  ``NAME``
    Name of output file

  Outputs:

  ``RES``
    Name of variable to store the full path of the stripped executable.

#]===]

function(compiler_generate_stripped_elf)
	set(options)
	set(oneValueArgs TARGET NAME RES)
	set(multiValueArgs)
	cmake_parse_arguments(MY "${options}" "${oneValueArgs}"
						"${multiValueArgs}" ${ARGN} )

	add_custom_command(
		TARGET ${MY_TARGET} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} --strip-unneeded
				$<TARGET_FILE:${MY_TARGET}>
				$<TARGET_FILE_DIR:${MY_TARGET}>/${MY_NAME})
	if (MY_RES)
		set(${MY_RES} $<TARGET_FILE_DIR:${MY_TARGET}>/${MY_NAME} PARENT_SCOPE)
	endif()
endfunction()

#[===[.rst:
.. cmake:command:: gcc_get_lib_location

  .. code-block:: cmake

    gcc_get_lib_location(TARGET foo NAME foo.stripped.elf RES var)

  Query the location of a specific library part of the GCC binary release. Can
  be used to find built in libraryes like libgcc.a when i.w. -nostdlib option
  is used.

  Inputs:

  ``LIBRARY_NAME``
    Name of the library to search for.

  Outputs:

  ``RES``
    Name of variable to store the full path of the library.

#]===]
function(gcc_get_lib_location)
	set(options)
	set(oneValueArgs LIBRARY_NAME RES)
	set(multiValueArgs)
	cmake_parse_arguments(MY "${options}" "${oneValueArgs}"
						"${multiValueArgs}" ${ARGN} )
	execute_process(
		COMMAND ${CMAKE_C_COMPILER} "--print-file-name=${MY_LIBRARY_NAME}"
		OUTPUT_VARIABLE _RES
		RESULT_VARIABLE _GCC_ERROR_CODE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

	if(_GCC_ERROR_CODE GREATER 0)
		message(WARNING "GCC (${CMAKE_C_COMPILER}) invocation failed, cannot determine location of library \"${MY_LIBRARY_NAME}\".")
		set(_RES "${LIBRARY_NAME}-NOTFOUND")
	endif()

	if (NOT IS_ABSOLUTE "${_RES}")
		message(WARNING "GCC (${CMAKE_C_COMPILER}) failed to return the location of file \"${MY_LIBRARY_NAME}\".")
		set(_RES "${LIBRARY_NAME}-NOTFOUND")
	endif()

	set(${MY_RES} ${_RES} PARENT_SCOPE)
endfunction()

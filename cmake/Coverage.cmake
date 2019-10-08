#
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[===[.rst:
Coverage CMake module
---------------------

Control flow
^^^^^^^^^^^^

Using the code coverage feature of the system starts with including
``Coverage`` module. This will implicitly check if all the requirements for
generating coverage are fulfilled. This includes checking the following
conditions.

- Compiler is GCC
- lcov executables exist
- ``c-picker-coverage-mapper`` is available

As the next step it sets the compiler flags to make GCC to generate binaries
with coverage information.


Variables
^^^^^^^^^

The module sets the following variables while it's checking its prerequisites.

.. cmake:variable:: LCOV_COMMAND

Path of lcov executable

.. cmake:variable:: GENHTML_COMMAND

Path of genhtml executable which is part of the lcov package.

.. cmake:variable:: CPICKER_COVERAGE_MAPPER_COMMAND

Path of ``c-picker-coverage-mapper`` executable which is provided by c-picker
pip package.


Functions
^^^^^^^^^

The module also contains functions for setting up the coverage feature.

#]===]

include_guard(DIRECTORY)

# Checking GCC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(FATAL_ERROR "Coverage measurement is only supported when using GCC")
endif()

# Checking lcov
find_program(LCOV_COMMAND "lcov")
if (NOT LCOV_COMMAND)
	message(FATAL_ERROR "Please install lcov")
endif()

# Checking c-picker-coverage-mapper
find_program(CPICKER_COVERAGE_MAPPER_COMMAND "c-picker-coverage-mapper")
if (NOT CPICKER_COVERAGE_MAPPER_COMMAND)
	message(FATAL_ERROR "Please install c-picker-coverage-mapper using pip (part of c-picker)")
endif()

# Checking genhtml
find_program(GENHTML_COMMAND "genhtml")
if (NOT GENHTML_COMMAND)
	message(FATAL_ERROR "Please install genhtml with genhtml (part of lcov)")
endif()

# Including this file enables code coverage measurement by adding the necessary compiler and
# linker flags.
set(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -fno-exceptions")
set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")

# Adding custom targets
add_custom_target(coverage)
add_custom_target(coverage_report)

# Adds a file to the dependency list of the target by inserting an accessory
# custom target. The name of the custom target is properly escaped.
function(add_coverage_dependency)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS TARGET DEPENDS)
	set(_MULTI_VALUE_ARGS)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	set(TARGET ${_MY_PARAMS_TARGET})
	set(DEPENDS ${_MY_PARAMS_DEPENDS})

	string(REGEX REPLACE "\\/" "_" CUSTOM_TARGET_SUFFIX ${DEPENDS})

	add_custom_target(${TARGET}_target_${CUSTOM_TARGET_SUFFIX} DEPENDS ${DEPENDS})
	add_dependencies(${TARGET} ${TARGET}_target_${CUSTOM_TARGET_SUFFIX})
endfunction()

#[===[.rst:
.. cmake:command:: coverage_generate

  .. code-block:: cmake

    coverage_generate(
    	NAME test_name
    	SOURCE_DIR source_directory
    	BINARY_DIR binary_directory
    	OUTPUT_FILE output_file
    )

  The function outputs an lcov info file for further processing. It also handles
  the remapping of the coverage of the c-picker generated files.

  Control flow:

  1. Running the ``lcov`` command for collecting the coverage data from the
     available ``.gcda`` and ``.gcno`` files in the ``BINARY_DIR``.

  2. The output of previous step is processed by ``c-picker-coverage-mapper``.
     This will remap the coverage of files in ``CPICKER_CACHE_PATH`` to the
     original source files.

  3. Adds the output file to the ``coverage`` target's dependency list.

  Inputs:

  ``NAME``
    Test name included in lcov info file

  ``SOURCE_DIR``
    Directory of source files

  ``BINARY_DIR``
    Directory of the ``.gcda`` and ``.gcno`` files

  ``OUTPUT_FILE``
    Output lcov coverage info file

  Global dependencies:

  ``CPICKER_CACHE_PATH``
    Root directory of the c-picker generated files


#]===]
function(coverage_generate)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS NAME SOURCE_DIR BINARY_DIR OUTPUT_FILE)
	set(_MULTI_VALUE_ARGS)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	set(TEST_NAME ${_MY_PARAMS_NAME})
	set(SOURCE_DIR ${_MY_PARAMS_SOURCE_DIR})
	set(BINARY_DIR ${_MY_PARAMS_BINARY_DIR})
	set(TEMP_FILE ${_MY_PARAMS_OUTPUT_FILE}_temp)
	set(OUTPUT_FILE ${_MY_PARAMS_OUTPUT_FILE})

	# Collecting information from .gcda and .gcno files into an lcov .info file
	# Mapping c-picker generated files' coverage info to the original source lines
	add_custom_command(
		OUTPUT ${TEMP_FILE} ${OUTPUT_FILE}
		COMMAND ${LCOV_COMMAND}
			--capture
			--test-name ${TEST_NAME}
			--directory ${BINARY_DIR}
			--base-directory ${SOURCE_DIR}
			--output-file ${TEMP_FILE}
		COMMAND ${CPICKER_COVERAGE_MAPPER_COMMAND}
			--input ${TEMP_FILE}
			--output ${OUTPUT_FILE}
			--mapping-path ${CPICKER_CACHE_PATH}
	)

	add_coverage_dependency(
		TARGET coverage
		DEPENDS ${OUTPUT_FILE}
	)
endfunction()

#[===[.rst:
.. cmake:command:: coverage_filter

  .. code-block:: cmake

    coverage_filter(
    	INPUT_FILE input_file
    	OUTPUT_FILE output_file
    	INCLUDE_DIRECTORY include_directory
    )

  The function filters the coverage data by including only the coverage of the
  files of ``INCLUDE_DIRECTORY`` or its subdirectories. It adds the filtered
  output file to the ``coverage`` target's dependency list.

  Inputs:

  ``INPUT_FILE``
    Input lcov coverage info file

  ``OUTPUT_FILE``
    Output lcov coverage info file

  ``INCLUDE_DIRECTORY``
    Root directory of included files

#]===]
function(coverage_filter)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS INPUT_FILE OUTPUT_FILE INCLUDE_DIRECTORY)
	set(_MULTI_VALUE_ARGS)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	set(INPUT_FILE ${_MY_PARAMS_INPUT_FILE})
	set(OUTPUT_FILE ${_MY_PARAMS_OUTPUT_FILE})
	set(INCLUDE_DIRECTORY ${_MY_PARAMS_INCLUDE_DIRECTORY})

	# The pattern must be an absolute path ending with an asterisk
	get_filename_component(INCLUDE_DIRECTORY_ABSPATH "${INCLUDE_DIRECTORY}" ABSOLUTE)
	set(INCLUDE_DIRECTORY_ABSPATH "${INCLUDE_DIRECTORY_ABSPATH}/*")

	add_custom_command(
		OUTPUT ${OUTPUT_FILE}
		COMMAND ${LCOV_COMMAND}
			--extract ${INPUT_FILE} \"${INCLUDE_DIRECTORY_ABSPATH}\"
			--output-file ${OUTPUT_FILE}
		DEPENDS ${INPUT_FILE}
	)

	add_coverage_dependency(
		TARGET coverage
		DEPENDS ${OUTPUT_FILE}
	)
endfunction()

#[===[.rst:
.. cmake:command:: coverage_generate_report

  .. code-block:: cmake

    coverage_generate_report(
    	INPUT_FILE input_file
    	OUTPUT_DIRECTORY output_directory
    )

  The function generates a HTML coverage report from the lcov info file into
  the ``OUTPUT_DIRECTORY``. It adds the output directory to the
  ``coverage_report`` target's dependency list.

  Inputs:

  ``INPUT_FILE``
    Input lcov coverage info file

  ``OUTPUT_DIRECTORY``
    Output directory of the coverage report where the ``index.html`` is the
    root file of the report.

#]===]
function(coverage_generate_report)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS INPUT_FILE OUTPUT_DIRECTORY)
	set(_MULTI_VALUE_ARGS)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	set(INPUT_FILE ${_MY_PARAMS_INPUT_FILE})
	set(OUTPUT_DIRECTORY ${_MY_PARAMS_OUTPUT_DIRECTORY})

	add_custom_command(
		OUTPUT ${OUTPUT_DIRECTORY}
		COMMAND genhtml ${INPUT_FILE}
			--show-details
			--output-directory ${OUTPUT_DIRECTORY}
		DEPENDS ${INPUT_FILE}
	)

	add_coverage_dependency(
		TARGET coverage_report
		DEPENDS ${OUTPUT_DIRECTORY}
	)
endfunction()

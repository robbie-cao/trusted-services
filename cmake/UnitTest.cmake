#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


#[===[.rst:
UnitTest CMake module
---------------------

Control flow
^^^^^^^^^^^^

1. Setting :cmake:variable:`CLANG_LIBRARY_PATH`

  1. Using :cmake:variable:`CLANG_LIBRARY_PATH` CMake variable

  2. Using ``CLANG_LIBRARY_PATH`` environment variable

  3. Trying to find by ``find_package`` function which calls :cmake:module:`FindLibClang`.

2. Checking if ``c-picker`` command is available


Variables
^^^^^^^^^

The module sets the following variables while it's checking its prerequisites.

.. cmake:variable:: GIT_COMMAND

Path of git executable.

.. cmake:variable:: CLANG_LIBRARY_PATH

c-picker uses libclang to parse the source files. If defined this variable
specifies the path of the library.

.. cmake:variable:: CPICKER_COMMAND

Path of c-picker executable which is part of the c-picker pip package.

.. cmake:variable:: UNIT_TEST_PROJECT_PATH

Path of the project source directory. **This needs to be specified
by the developer** to point to a suitable working copy of project to be tested.

.. cmake:variable:: CPPUTEST_URL

URL of the CppUTest git repository. By default it points to the official Github
repository of CppUTest. It can be used to specify a different CppUTest mirror.

.. cmake:variable:: CPPUTEST_REFSPEC

CppUTest git refspec. The default value selects the latest release.

.. cmake:variable:: CPPUTEST_INSTALL_PATH

Temporary directory used during CppUTest build

.. cmake:variable:: CPPUTEST_PACKAGE_PATH

Path of the CppUTest CMake package directory

.. cmake:variable:: CPICKER_CACHE_PATH

Directory of c-picker generated files. Subdirectories are added according to
the path of the original source file's path.

.. cmake:variable:: UNIT_TEST_COMMON_SOURCES

Lists of source files that are included in all test builds.


Functions
^^^^^^^^^

#]===]

include_guard(DIRECTORY)

include(FetchContent)

set(CLANG_LIBRARY_PATH_HELP "libclang directory for c-picker")

set(CPPUTEST_URL "https://github.com/cpputest/cpputest.git" CACHE STRING "CppUTest repository URL")
set(CPPUTEST_REFSPEC "v4.0" CACHE STRING "CppUTest git refspec")
set(CPPUTEST_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/CppUTest_install CACHE PATH "CppUTest installation directory")
set(CPPUTEST_PACKAGE_PATH ${CPPUTEST_INSTALL_PATH}/lib/CppUTest/cmake CACHE PATH "CppUTest CMake package directory")

set(CPICKER_CACHE_PATH ${CMAKE_CURRENT_BINARY_DIR}/cpicker_cache CACHE PATH "Directory of c-picker generated file")

set(UNIT_TEST_COMMON_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../common/main.cpp CACHE STRING "List of common test source files")

if (POSITION_INDEPENDENT_CODE)
	string(APPEND CPPUTEST_CXX_FLAGS -fPIC " " -pie)
	string(APPEND CPPUTEST_C_FLAGS -fPIC " " -pie)
	SET(TEST_COMPILE_OPTIONS ${TEST_COMPILE_OPTIONS} -fPIC -pie)
	SET(TEST_LINK_OPTIONS ${TEST_LINK_OPTIONS} -fPIC -pie)
else()
	string(APPEND CPPUTEST_CXX_FLAGS -no-pie)
	string(APPEND CPPUTEST_C_FLAGS -no-pie)
	string(APPEND TEST_COMPILE_OPTIONS -no-pie)
	string(APPEND TEST_LINK_OPTIONS -no-pie)
endif()

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

if (DEFINED CLANG_LIBRARY_PATH)
	message(STATUS "Using CLANG_LIBRARY_PATH from CMake variable (command line or cache)")

	if (DEFINED ENV{CLANG_LIBRARY_PATH})
		if (NOT (${CLANG_LIBRARY_PATH} STREQUAL $ENV{CLANG_LIBRARY_PATH}))
			message(WARNING "Both CLANG_LIBRARY_PATH CMake and environment variables are set but have different values")
		endif()
	endif()
else()
	if (DEFINED ENV{CLANG_LIBRARY_PATH})
		message(STATUS "Setting CLANG_LIBRARY_PATH based on environment variable")
		set(CLANG_LIBRARY_PATH $ENV{CLANG_LIBRARY_PATH} CACHE PATH ${CLANG_LIBRARY_PATH_HELP})
	else()
		message(STATUS "Setting CLANG_LIBRARY_PATH based on find_package")
		find_package(LibClang REQUIRED)
		set(CLANG_LIBRARY_PATH ${LibClang_LIBRARY_DIRS} CACHE PATH ${CLANG_LIBRARY_PATH_HELP})
	endif()
endif()

message(STATUS "CLANG_LIBRARY_PATH has been set to ${CLANG_LIBRARY_PATH}")

# Checking c-picker
find_program(CPICKER_COMMAND "c-picker")
if (NOT CPICKER_COMMAND)
	message(FATAL_ERROR "Please install c-picker using pip")
endif()

#[===[.rst:
.. cmake:command:: unit_test_init_cpputest

  .. code-block:: cmake

    unit_test_init_cpputest()

  The ``unit_test_init_cpputest`` CMake function fetches and build CppUTest unit testing framework.
  It also enables linking the library to the test binaries.

  Global dependencies:

  ``CPPUTEST_URL``
    Root directory of the c-picker generated files

  ``CPPUTEST_REFSPEC``
    Common source files for every test build

#]===]
function(unit_test_init_cpputest)
	# Fetching CppUTest
	FetchContent_Declare(
		cpputest
		GIT_REPOSITORY ${CPPUTEST_URL}
		GIT_TAG ${CPPUTEST_REFSPEC}
		GIT_SHALLOW TRUE
		PATCH_COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/common/cpputest-cmake-fix.patch || true
	)

	# FetchContent_GetProperties exports cpputest_SOURCE_DIR and cpputest_BINARY_DIR variables
	FetchContent_GetProperties(cpputest)
	if(NOT cpputest_POPULATED)
		message(STATUS "Fetching CppUTest")
		FetchContent_Populate(cpputest)
	endif()

	# Build and install CppUTest in CMake time. This makes us able to use CppUTest as a CMake package later.
	# Memory leak detection is turned off to avoid conflict with memcheck.
	execute_process(COMMAND
		${CMAKE_COMMAND}
			-DMEMORY_LEAK_DETECTION=OFF
			-DLONGLONG=ON
			-DC++11=ON
			-DCMAKE_INSTALL_PREFIX=${CPPUTEST_INSTALL_PATH}
			-DCPPUTEST_CXX_FLAGS=${CPPUTEST_CXX_FLAGS}
			-DCPPUTEST_C_FLAGS=${CPPUTEST_C_FLAGS}
			-GUnix\ Makefiles
			${cpputest_SOURCE_DIR}
		WORKING_DIRECTORY
			${cpputest_BINARY_DIR}
	)
	execute_process(COMMAND ${CMAKE_COMMAND} --build ${cpputest_BINARY_DIR} -- install -j)

	# Finding CppUTest package. CMake will check [package name]_DIR variable.
	set(CppUTest_DIR ${CPPUTEST_PACKAGE_PATH} CACHE PATH "Path of CppUTestConfig.cmake")
	find_package(CppUTest CONFIG REQUIRED)

	# find_package sets the CppUTest_INCLUDE_DIRS and CppUTest_LIBRARIES variables
	include_directories(${CppUTest_INCLUDE_DIRS})
	link_libraries(${CppUTest_LIBRARIES})
endfunction()

#[===[.rst:
.. cmake:command:: unit_test_add_suite

  .. code-block:: cmake

    unit_test_add_suite(
    	NAME test_name
    	SOURCES source_files
    	INCLUDE_DIRECTORIES include_directories
    	COMPILE_DEFINITIONS defines
    	DEPENDS dependencies
    )

  The ``unit_test_add_suite`` CMake function provides a convenient interface for
  defining unit test suites. Basically its input is the test source files, include
  paths and macro definitions and it internally does all the necessary steps to
  have the test binary registered in CTest as a result.

  Control flow:

  1. Adding new executable named ``NAME``

  2. Iterating throught ``SOURCES``

     1. If it's a normal source file add to the executable's source list
     2. If it's a YAML file add as a c-picker custom command and add the generated
        file to the executable's source list

  3. Setting include directories

  4. Setting defines

  5. Adding extra dependencies of the test build

  6. Adding executable to the system as a test

  Inputs:

  ``NAME``
    Unique name of the test suite

  ``SOURCES`` (multi, optional)
    Source files

  ``INCLUDE_DIRECTORIES`` (multi, optional)
    Include directories

  ``COMPILE_DEFINITIONS`` (multi, optional)
    Defines

  ``DEPENDS`` (multi, optional)
    Extra targets as dependencies of the test build

  Global dependencies:

  ``UNIT_TEST_PROJECT_PATH``
    Root directory of the project under test.

  ``CPICKER_CACHE_PATH``
    Root directory of the c-picker generated files

  ``UNIT_TEST_COMMON_SOURCES``
    Common source files for every test build

  ``CTest``
    Built-in testing module of CMake

#]===]
function(unit_test_add_suite)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS NAME)
	set(_MULTI_VALUE_ARGS SOURCES INCLUDE_DIRECTORIES COMPILE_DEFINITIONS DEPENDS)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	if(NOT DEFINED BUILD_TESTING)
		message(FATAL_ERROR
			"unit_test_add_suite(): "
			"CTest module should be included in the root CMakeLists.txt before calling this function.")
	endif()

	if (NOT UNIT_TEST_PROJECT_PATH)
		message(FATAL_ERROR "UNIT_TEST_PROJECT_PATH is not set")
	endif()

	set(TEST_NAME ${_MY_PARAMS_NAME})
	set(TEST_INCLUDE_DIRECTORIES ${_MY_PARAMS_INCLUDE_DIRECTORIES})
	set(TEST_COMPILE_DEFINITIONS ${_MY_PARAMS_COMPILE_DEFINITIONS})
	set(TEST_DEPENDS ${_MY_PARAMS_DEPENDS})

	add_executable(${TEST_NAME} ${UNIT_TEST_COMMON_SOURCES})

	foreach(TEST_SOURCE ${_MY_PARAMS_SOURCES})
		get_filename_component(TEST_SOURCE_EXTENSION ${TEST_SOURCE} EXT)

		if (${TEST_SOURCE_EXTENSION} STREQUAL ".yml")
			# Building output file name: tests/a/b/test.yml -> ${CPICKER_CACHE_PATH}/a/b/test.c
			get_filename_component(TEST_SOURCE_DIR ${TEST_SOURCE} DIRECTORY)
			file(RELATIVE_PATH CPICKER_SOURCE_DIR ${UNIT_TEST_PROJECT_PATH} ${TEST_SOURCE_DIR})
			get_filename_component(TEST_SOURCE_NAME ${TEST_SOURCE} NAME_WE)
			set(CPICKER_OUTPUT ${CPICKER_CACHE_PATH}/${TEST_NAME}/${CPICKER_SOURCE_DIR}/${TEST_SOURCE_NAME}.c)

			# Creating output directory
			get_filename_component(OUTPUT_DIRECTORY ${CPICKER_OUTPUT} DIRECTORY)
			file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})

			# Fetching referenced source files as the dependencies of the generated file
			execute_process(
				COMMAND
					${CMAKE_COMMAND} -E env CLANG_LIBRARY_PATH=${CLANG_LIBRARY_PATH}
					${CPICKER_COMMAND} --config ${TEST_SOURCE} --root ${UNIT_TEST_PROJECT_PATH} --print-dependencies
				OUTPUT_VARIABLE CPICKER_DEPENDENCIES
			)

			# Adding custom command for invoking c-picker
			add_custom_command(
				OUTPUT ${CPICKER_OUTPUT}
				COMMAND
					${CMAKE_COMMAND} -E env CLANG_LIBRARY_PATH=${CLANG_LIBRARY_PATH}
					${CPICKER_COMMAND} --config ${TEST_SOURCE} --root ${UNIT_TEST_PROJECT_PATH} > ${CPICKER_OUTPUT}
				DEPENDS ${TEST_SOURCE} ${CPICKER_DEPENDENCIES}
				COMMENT "Generating c-picker output ${CPICKER_OUTPUT}"
			)
			set(TEST_SOURCE ${CPICKER_OUTPUT})
		endif()

		target_sources(${TEST_NAME} PRIVATE ${TEST_SOURCE})
	endforeach()

	target_include_directories(${TEST_NAME} PRIVATE ${TEST_INCLUDE_DIRECTORIES})
	target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_COMPILE_DEFINITIONS})
	target_compile_options(${TEST_NAME} PRIVATE ${TEST_COMPILE_OPTIONS})
	target_link_options(${TEST_NAME} PRIVATE ${TEST_LINK_OPTIONS})
	if (TEST_DEPENDS)
		add_dependencies(${TEST_NAME} ${TEST_DEPENDS})
	endif()
	add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

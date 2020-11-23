#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
NonoPB integration for cmake
----------------------------

This module will:
	- download nanopb if not available locally
	- build the runtime static library and the generator
	- import the static library to the build
	- define a function to provide access to the generator

Note: the python module created by the generator build will be installed under
Python_SITELIB ("Third-party platform independent installation directory.")
This means the build may alter the state of your system. Please use virtualenv.

Note: see requirements.txt for dependnecies which need to be installed before
running this module.

#]===]

#### Get the dependency

set(NANOPB_URL "https://github.com/nanopb/nanopb.git" CACHE STRING "nanopb repository URL")
set(NANOPB_REFSPEC "nanopb-0.4.2" CACHE STRING "nanopb git refspec")
set(NANOPB_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/nanopb_install" CACHE PATH "nanopb installation directory")
set(NANOPB_PACKAGE_PATH "${NANOPB_INSTALL_PATH}/libnanopb/cmake" CACHE PATH "nanopb CMake package directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching nanopb
FetchContent_Declare(
	nanopb
	GIT_REPOSITORY ${NANOPB_URL}
	GIT_TAG ${NANOPB_REFSPEC}
	GIT_SHALLOW TRUE
	#See the .patch file for details on why it is needed.
	PATCH_COMMAND git stash
		COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/fix-pyhon-name.patch
)

# FetchContent_GetProperties exports nanopb_SOURCE_DIR and nanopb_BINARY_DIR variables
FetchContent_GetProperties(nanopb)
if(NOT nanopb_POPULATED)
	message(STATUS "Fetching nanopb")
	FetchContent_Populate(nanopb)
endif()

#### Build the runtime and the generator.
if( NOT CMAKE_CROSSCOMPILING)
	execute_process(COMMAND
		${CMAKE_COMMAND}
				-DBUILD_SHARED_LIBS=Off
				-DBUILD_STATIC_LIBS=On
				-Dnanopb_BUILD_RUNTIME=On
				-Dnanopb_BUILD_GENERATOR=On
				-Dnanopb_PROTOC_PATH=${nanopb_SOURCE_DIR}/generator/protoc
				-Dnanopb_MSVC_STATIC_RUNTIME=Off
				-DCMAKE_INSTALL_PREFIX=${NANOPB_INSTALL_PATH}
				-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
				-GUnix\ Makefiles
				${nanopb_SOURCE_DIR}
			WORKING_DIRECTORY
				${nanopb_BINARY_DIR}
			RESULT_VARIABLE _exec_error
	)
else()
	execute_process(COMMAND
		${CMAKE_COMMAND}
				-DBUILD_SHARED_LIBS=Off
				-DBUILD_STATIC_LIBS=On
				-Dnanopb_BUILD_RUNTIME=On
				-Dnanopb_BUILD_GENERATOR=On
				-Dnanopb_PROTOC_PATH=${nanopb_SOURCE_DIR}/generator/protoc
				-Dnanopb_MSVC_STATIC_RUNTIME=Off
				-DCMAKE_INSTALL_PREFIX=${NANOPB_INSTALL_PATH}
				-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
				-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
				-GUnix\ Makefiles
				${nanopb_SOURCE_DIR}
			WORKING_DIRECTORY
				${nanopb_BINARY_DIR}
			RESULT_VARIABLE _exec_error
	)
endif()

if (_exec_error)
	message(FATAL_ERROR "Configuration step of nanopb runtime failed with ${_exec_error}.")
endif()

execute_process(COMMAND
		${CMAKE_COMMAND} --build ${nanopb_BINARY_DIR} -- install -j8
		RESULT_VARIABLE _exec_error
	)
if (_exec_error)
	message(FATAL_ERROR "Build step of nanopb runtime failed with ${_exec_error}.")
endif()

#### Include Nanopb runtime in the build.
find_package(Nanopb
			PATHS "${NANOPB_INSTALL_PATH}"
			NO_DEFAULT_PATH
		)

#### Build access to the protobuf compiler
#TODO: verify protoc dependencies: python3-protobuf
find_package(Python3 COMPONENTS Interpreter)

if (NOT Python3_Interpreter_FOUND)
	message(FATAL_ERROR "Failed to find python3 interpreter.")
endif()

find_file(NANOPB_GENERATOR_PATH
			NAMES nanopb_generator.py
			PATHS ${nanopb_SOURCE_DIR}/generator
			DOC "nanopb protobuf compiler"
			NO_DEFAULT_PATH
		)

if (NOT NANOPB_GENERATOR_PATH)
	message(FATAL_ERROR "Nanopb generator was not found!")
endif()

#[===[.rst:
.. cmake:command:: protobuf_generate

  .. code-block:: cmake

	 protobuf_generate(SRC file.proto
					 TGT foo
					 NAMESPACE bar
					 BASE_DIR "proto/definitions")

  Run the ``nanopb_generator`` to compile a protobuf definition file into C source.
  Generated source file will be added to the source list of ``TGT``. Protobuf
  compilation will take part before TGT+NAMESPACE is built.

  Protobuf file names added to the same TGT must not collide.

  Inputs:

  ``SRC``
	Path to of the protobuf file to process. Either absoluto or relative to the
	callers location.

  ``TGT``
	Name of target to compile generated source files.

  ``NAMESPACE``
	Namespace to put generated files under. Specifies include path and allows
	separating colliding protobuf files.

  ``BASE_DIR``
	Base directory. Generated files are located reletive to this base.

#]===]
function(protobuf_generate)
	set(_options )
	set(_oneValueArgs SRC TGT NAMESPACE BASE_DIR)
	set(_multiValueArgs )

	cmake_parse_arguments(PARAMS "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

	#Verify mandatory parameters
	if (NOT DEFINED PARAMS_SRC)
		message(FATAL_ERROR "nanopb_generate(): mandatory parameter SRC missing.")
	endif()
	if (NOT DEFINED PARAMS_TGT)
		message(FATAL_ERROR "nanopb_generate(): mandatory parameter TGT missing.")
	endif()
	if (NOT DEFINED PARAMS_NAMESPACE)
		message(FATAL_ERROR "nanopb_generate(): mandatory parameter NAMESPACE missing.")
	endif()
	if (NOT DEFINED PARAMS_BASE_DIR)
		message(FATAL_ERROR "nanopb_generate(): mandatory parameter BASE_DIR missing.")
	endif()

	#If SRC is not abolute make it relative to the callers location.
	if (NOT IS_ABSOLUTE ${PARAMS_SRC})
		set(PARAMS_SRC "${CMAKE_CURRENT_LIST_DIR}/${PARAMS_SRC}")
	endif()

	#Calculate the output directory
	set(_OUT_DIR_BASE ${CMAKE_BINARY_DIR}/src/${PARAMS_NAMESPACE})
	#Calculate output file names
	get_filename_component(_BASENAME ${PARAMS_SRC} NAME_WE)

	#Get relative path or SRC to BASE_DIR
	file(RELATIVE_PATH _SRC_REL ${PARAMS_BASE_DIR} ${PARAMS_SRC})
	get_filename_component(_OUT_DIR_REL ${_SRC_REL} DIRECTORY )

	#Calculate output file paths
	set(_OUT_C "${_OUT_DIR_BASE}/${_OUT_DIR_REL}/${_BASENAME}.pb.c")
	set(_OUT_H "${_OUT_DIR_BASE}/${_OUT_DIR_REL}/${_BASENAME}.pb.h")

	#some helper variables for the purpose of readability
	set(_nanopb_target "nanopb_generate_${PARAMS_TGT}_${PARAMS_NAMESPACE}")
	set(_nanopb_fake_file "nanopb_generate_ff_${PARAMS_TGT}")

	if (NOT TARGET "${_nanopb_target}")
		#Create a custom target which depends on a "fake" file.
		add_custom_target("${_nanopb_target}"
							DEPENDS "${_nanopb_fake_file}")
		#Tell cmake the dependency (source) file is fake.
		set_source_files_properties("${_nanopb_fake_file}" PROPERTIES SYMBOLIC "true")
		#Add a cutom command to the target to create output directory.
		add_custom_command(OUTPUT "${_nanopb_fake_file}"
			COMMAND ${CMAKE_COMMAND} -E make_directory ${_OUT_DIR_BASE}
			COMMENT "Generating source from protobuf definitions for target ${PARAMS_TGT}")
		#Ensure protobuf build happens before test target.
		add_dependencies(${PARAMS_TGT} ${_nanopb_target})
		#Add include path to protobuf output.
		target_include_directories(${PARAMS_TGT} PRIVATE ${_OUT_DIR_BASE})
	endif()

	#Append a protobuf generator command to the nanopb_generate target.
	add_custom_command(OUTPUT "${_nanopb_fake_file}" "${_OUT_C}" "${_OUT_H}"
					   APPEND
					   COMMAND ${Python3_EXECUTABLE} ${NANOPB_GENERATOR_PATH}
						  -I ${PARAMS_BASE_DIR}
						  -D ${_OUT_DIR_BASE}
						  ${_SRC_REL}
					   DEPENDS "${PARAMS_SRC}")

	#Add generated file to the target
	set_property(SOURCE "${_OUT_C}" PROPERTY GENERATED TRUE)
	target_sources(${PARAMS_TGT} PRIVATE "${_OUT_C}")
endfunction()

#[===[.rst:
.. cmake:command:: protobuf_generate_all

  .. code-block:: cmake

	 protobuf_generate_all(TGT foo
					 NAMESPACE bar
					 BASE_DIR "proto/definitions")

  Generates C code from all .proto files listed in the target
  property PROTOBUF_FILES.

  Inputs:

   ``TGT``
	Name of target to compile generated source files.

  ``NAMESPACE``
	Namespace to put generated files under. Specifies include path and allows
	separating colliding protobuf files.

  ``BASE_DIR``
	Base directory. Generated files are located reletive to this base.

#]===]
function(protobuf_generate_all)
	set(_options )
	set(_oneValueArgs TGT NAMESPACE BASE_DIR)
	set(_multiValueArgs )

	cmake_parse_arguments(PARAMS "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

	#Verify mandatory parameters
	if (NOT DEFINED PARAMS_TGT)
		message(FATAL_ERROR "nanopb_generate_all(): mandatory parameter TGT missing.")
	endif()
	if (NOT DEFINED PARAMS_NAMESPACE)
		message(FATAL_ERROR "nanopb_generate_all(): mandatory parameter NAMESPACE missing.")
	endif()
	if (NOT DEFINED PARAMS_BASE_DIR)
		message(FATAL_ERROR "nanopb_generate_all(): mandatory parameter BASE_DIR missing.")
	endif()

	get_property(_protolist TARGET ${PARAMS_TGT} PROPERTY PROTOBUF_FILES)

	#Build of each .proto file
	foreach(_file IN LISTS _protolist)
		protobuf_generate(
				TGT ${PARAMS_TGT}
				SRC "${_file}"
				NAMESPACE ${PARAMS_NAMESPACE}
				BASE_DIR ${PARAMS_BASE_DIR})
	endforeach()
endfunction()
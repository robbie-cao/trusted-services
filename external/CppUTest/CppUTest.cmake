#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(CPPUTEST_URL "https://github.com/cpputest/cpputest.git" CACHE STRING "CppUTest repository URL")
set(CPPUTEST_REFSPEC "v3.8" CACHE STRING "CppUTest git refspec")
set(CPPUTEST_INSTALL_PATH ${CMAKE_CURRENT_BINARY_DIR}/CppUTest_install CACHE PATH "CppUTest installation directory")

include(FetchContent)

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching CppUTest
FetchContent_Declare(
	cpputest
	GIT_REPOSITORY ${CPPUTEST_URL}
	GIT_TAG ${CPPUTEST_REFSPEC}
	GIT_SHALLOW TRUE
	PATCH_COMMAND git stash
		COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/cpputest-cmake-fix.patch
)

# FetchContent_GetProperties exports cpputest_SOURCE_DIR and cpputest_BINARY_DIR variables
FetchContent_GetProperties(cpputest)
if(NOT cpputest_POPULATED)
	message(STATUS "Fetching CppUTest")
	FetchContent_Populate(cpputest)
endif()

# Build and install CppUTest configuration time. This makes us able to use CppUTest as a CMake package.
# Memory leak detection is turned off to avoid conflict with memcheck.
if(NOT CMAKE_CROSSCOMPILING)
	execute_process(COMMAND
		${CMAKE_COMMAND}
			-DMEMORY_LEAK_DETECTION=OFF
			-DLONGLONG=ON
			-DC++11=ON
			-DCMAKE_INSTALL_PREFIX=${CPPUTEST_INSTALL_PATH}
			-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
			-G${CMAKE_GENERATOR}
			${cpputest_SOURCE_DIR}
		WORKING_DIRECTORY
			${cpputest_BINARY_DIR}
			RESULT_VARIABLE
				_exec_error
	)
else()
	execute_process(COMMAND
	${CMAKE_COMMAND}
		-DMEMORY_LEAK_DETECTION=OFF
		-DLONGLONG=ON
		-DC++11=ON
		-DCMAKE_INSTALL_PREFIX=${CPPUTEST_INSTALL_PATH}
		-DCMAKE_TOOLCHAIN_FILE=${TS_EXTERNAL_LIB_TOOLCHAIN_FILE}
		-DTESTS=OFF
		-DEXTENSIONS=OFF
		-DHAVE_FORK=OFF
		-DCPP_PLATFORM=armcc
		-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
		-G${CMAKE_GENERATOR}
		${cpputest_SOURCE_DIR}
	WORKING_DIRECTORY
		${cpputest_BINARY_DIR}
	RESULT_VARIABLE
		_exec_error
	)
endif()
if (NOT _exec_error EQUAL 0)
	message(FATAL_ERROR "Configuriong CppUTest build failed.")
endif()
execute_process(COMMAND
	${CMAKE_COMMAND}
		--build ${cpputest_BINARY_DIR}
		-- install -j8
	RESULT_VARIABLE
		_exec_error
	)
if (NOT _exec_error EQUAL 0)
	message(FATAL_ERROR "Building CppUTest failed.")
endif()

# Finding CppUTest package. CMake will check [package name]_DIR variable.
set(CppUTest_DIR ${CPPUTEST_INSTALL_PATH}/lib/CppUTest/cmake CACHE PATH "CppUTest package location" FORCE)
find_package(CppUTest CONFIG REQUIRED NO_DEFAULT_PATH PATHS ${CppUTest_DIR})
# CppUTest package files do not set include path properties on the targets.
# Fix this here.
foreach(_cpputest_target IN LISTS CppUTest_LIBRARIES)
	if (TARGET  ${_cpputest_target})
		target_include_directories(${_cpputest_target} INTERFACE ${CppUTest_INCLUDE_DIRS})
		target_compile_features(${_cpputest_target} INTERFACE cxx_std_11)
	endif()
endforeach()

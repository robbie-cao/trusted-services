#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Import libts into a dependent in-tree deployment build.  Where another
# deployment uses libts, including this file in the dependent deployment
# CMake build file allows libts to be built and installed into the binary
# directory of the dependent.
#-------------------------------------------------------------------------------
option(CFG_FORCE_PREBUILT_LIBTS Off)
# Try to find a pre-build package.
find_package(libts "1.0.0" QUIET PATHS ${CMAKE_CURRENT_BINARY_DIR}/libts_install/${TS_ENV}/lib/cmake/libts)
if(NOT libts_FOUND)
	if (CFG_FORCE_PREBUILT_LIBTS)
		string(CONCAT _msg "find_package() failed to find the \"libts\" package. Please pass -Dlibts_ROOT=<path> or"
						   " -DCMAKE_FIND_ROOT_PATH=<path> cmake variable, where <path> is the INSTALL_PREFIX used"
						   " when building libts. libts_ROOT can be set in the environment too."
						   "If you wish to debug the search process pass -DCMAKE_FIND_DEBUG_MODE=ON to cmake.")
		message(FATAL_ERROR ${_msg})
	endif()
	# If not successful, build libts as a sub-project.
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env "CROSS_COMPILE=${CROSS_COMPILE}"
		${CMAKE_COMMAND}
			-S ${TS_ROOT}/deployments/libts/${TS_ENV}
			-B ${CMAKE_CURRENT_BINARY_DIR}/libts
		RESULT_VARIABLE
			_exec_error
		)
	if (NOT _exec_error EQUAL 0)
		message(FATAL_ERROR "Configuring libts failed. ${_exec_error}")
	endif()
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env "CROSS_COMPILE=${CROSS_COMPILE}"
		${CMAKE_COMMAND}
			--build ${CMAKE_CURRENT_BINARY_DIR}/libts
			--parallel ${PROCESSOR_COUNT}
		RESULT_VARIABLE
			_exec_error
		)
	if (NOT _exec_error EQUAL 0)
		message(FATAL_ERROR "Installing libts failed. ${_exec_error}")
	endif()
	execute_process(COMMAND
		${CMAKE_COMMAND} -E env "CROSS_COMPILE=${CROSS_COMPILE}"
		${CMAKE_COMMAND}
			--install ${CMAKE_CURRENT_BINARY_DIR}/libts
			--prefix ${CMAKE_CURRENT_BINARY_DIR}/libts_install
		RESULT_VARIABLE
			_exec_error
		)
	if (NOT _exec_error EQUAL 0)
		message(FATAL_ERROR "Installing libts failed. ${_exec_error}")
	endif()

	install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/libts/cmake_install.cmake)

	find_package(libts "1.0.0" QUIET REQUIRED PATHS ${CMAKE_CURRENT_BINARY_DIR}/libts_install/${TS_ENV}/lib/cmake/libts)
else()
	message(STATUS "Using prebuilt libts from ${libts_DIR}")
endif()

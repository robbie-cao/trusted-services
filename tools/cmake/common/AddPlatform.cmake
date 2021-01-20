#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
Add platform provided components to a build
-------------------------------------------

#]===]


#[===[.rst:
.. cmake:command:: add_platform

	.. code:: cmake

		add_platform(TARGET <target name>)

	INPUTS:

	``TARGET``
	The name of an already defined target to add platform components to.

#]===]
function(add_platform)
	set(options  )
	set(oneValueArgs TARGET)
	cmake_parse_arguments(MY_PARAMS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	if(NOT DEFINED MY_PARAMS_TARGET)
		message(FATAL_ERROR "add_platform: mandatory parameter TARGET not defined!")
	endif()

	set(TGT ${MY_PARAMS_TARGET} CACHE STRING "")

	# Ensure file path conforms to lowercase project convention
	string(TOLOWER "${TS_PLATFORM_ROOT}/${TS_PLATFORM}/platform.cmake" _platdef)
	include(${_platdef})
	set(CMAKE_CONFIGURE_DEPENDS ${_platdef})

	unset(TGT CACHE)
endfunction()

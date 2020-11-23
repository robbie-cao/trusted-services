#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
.. cmake:command:: export_library

	.. code:: cmake

		export_library(TARGET LIB_NAME INTERFACE_FILES)

	INPUTS:

	``TARGET``
	The name of an already defined target that corresponds to the library.

	``LIB_NAME``
	The name of the library.

	``INTERFACE_FILES``
	List of header files to declare the library's public interface.

#]===]
function(export_library)
	set(options  )
	set(oneValueArgs TARGET LIB_NAME)
	set(multiValueArgs INTERFACE_FILES)
	cmake_parse_arguments(MY_PARAMS "${options}" "${oneValueArgs}"
						"${multiValueArgs}" ${ARGN} )

	if(NOT DEFINED MY_PARAMS_TARGET)
		message(FATAL_ERROR "export_library: mandatory parameter TARGET not defined!")
	endif()
	if(NOT DEFINED MY_PARAMS_LIB_NAME)
		message(FATAL_ERROR "export_library: mandatory parameter LIB_NAME not defined!")
	endif()

	# Set default install location if none specified
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
	endif()

	# Specify export name and destinations for install
	install(
		TARGETS ${MY_PARAMS_TARGET}
		EXPORT ${MY_PARAMS_LIB_NAME}_targets
		ARCHIVE
			DESTINATION lib
		LIBRARY
			DESTINATION lib
		PUBLIC_HEADER
			DESTINATION include
	)

	# Install library header files files
	install(
		FILES ${MY_PARAMS_INTERFACE_FILES}
		DESTINATION include
	)

	# Install the export details
	install(
		EXPORT ${MY_PARAMS_LIB_NAME}_targets
		FILE ${MY_PARAMS_LIB_NAME}_targets.cmake
		NAMESPACE ${MY_PARAMS_LIB_NAME}::
		DESTINATION lib/cmake
		COMPONENT ${MY_PARAMS_LIB_NAME}
	)
endfunction()

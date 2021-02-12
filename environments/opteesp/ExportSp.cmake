#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

foreach(_var IN ITEMS EXPORT_SP_NAME EXPORT_SP_UUID)
	if(NOT DEFINED ${_var})
		message(FATAL_ERROR
				"Input variable ${_var} is undefined! Please define it"
				"using set(${_var} ...) before including this file.")
	endif()
endforeach()

configure_file(${CMAKE_CURRENT_LIST_DIR}/sp.mk.in ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_NAME}.mk @ONLY NEWLINE_STYLE UNIX)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_NAME}.mk DESTINATION ${TS_ENV}/lib/make)

unset(EXPORT_SP_NAME)
unset(EXPORT_SP_UUID)

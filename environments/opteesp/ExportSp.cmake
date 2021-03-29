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

get_filename_component(PARENT_LIST_DIR ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
string(REGEX REPLACE
	"([a-f0-9]+)-([a-f0-9]+)-([a-f0-9]+)-([a-f0-9]+)-([a-f0-9][a-f0-9][a-f0-9][a-f0-9])([a-f0-9]+)"
	"0x\\1 0x\\2\\3 0x\\4\\5 0x\\6"
	EXPORT_SP_UUID_DT ${EXPORT_SP_UUID})

set(DTS_TAG "")
set(DTS_NODE "${EXPORT_SP_NAME}")
configure_file(${PARENT_LIST_DIR}/default_${EXPORT_SP_NAME}.dts.in
	${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dtsi @ONLY NEWLINE_STYLE UNIX)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dtsi DESTINATION ${TS_ENV}/manifest)

set(DTS_TAG "/dts-v1/;")
set(DTS_NODE "/")
configure_file(${PARENT_LIST_DIR}/default_${EXPORT_SP_NAME}.dts.in
	${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dts @ONLY NEWLINE_STYLE UNIX)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dts DESTINATION ${TS_ENV}/manifest)

unset(DTS_TAG)
unset(DTS_NODE)
unset(PARENT_LIST_DIR)
unset(EXPORT_SP_UUID_DT)
unset(EXPORT_SP_NAME)
unset(EXPORT_SP_UUID)

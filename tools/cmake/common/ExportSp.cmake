#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
.. cmake:command:: export_sp

	.. code:: cmake

		export_sp(SP_UUID <uuid> SP_NAME <name> MK_IN <.mk path> DTS_IN <DTS path> JSON_IN <JSON path>)

	INPUTS:

	``SP_UUID``
	The UUID of the SP as a string.

	``SP_NAME``
	The name of the SP.

	``MK_IN``
	Optional, Makefile template for OP-TEE build

	``DTS_IN``
	Manifest file template

	``JSON_IN``
	Optional, SP layout JSON file template for TF-A

#]===]
function (export_sp)
	set(options)
	set(oneValueArgs SP_UUID SP_NAME MK_IN DTS_IN JSON_IN)
	set(multiValueArgs)
	cmake_parse_arguments(EXPORT "${options}" "${oneValueArgs}"
						"${multiValueArgs}" ${ARGN} )

	if(NOT DEFINED EXPORT_SP_UUID)
		message(FATAL_ERROR "export_sp: mandatory parameter SP_UUID not defined!")
	endif()
	if(NOT DEFINED EXPORT_SP_NAME)
		message(FATAL_ERROR "export_sp: mandatory parameter SP_NAME not defined!")
	endif()
	if(NOT DEFINED EXPORT_DTS_IN)
		message(FATAL_ERROR "export_sp: mandatory parameter DTS_IN not defined!")
	endif()

	if (DEFINED EXPORT_MK_IN)
		configure_file(${EXPORT_MK_IN} ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_NAME}.mk @ONLY NEWLINE_STYLE UNIX)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_NAME}.mk DESTINATION ${TS_ENV}/lib/make)
	endif()

	# Converting UUID from the canonical string format to four 32 bit unsigned integers.
	# The first byte of the UUID is the MSB of the first integer.
	# 01234567-89ab-cdef-0123-456789abcdef -> 0x01234567 0x89abcdef 0x01234567 0x89abcdef
	string(REGEX REPLACE
		"([a-f0-9]+)-([a-f0-9]+)-([a-f0-9]+)-([a-f0-9]+)-([a-f0-9][a-f0-9][a-f0-9][a-f0-9])([a-f0-9]+)"
		"0x\\1 0x\\2\\3 0x\\4\\5 0x\\6"
		EXPORT_SP_UUID_DT ${EXPORT_SP_UUID})

	# As the .dtsi is meant to be included in .dts file, it shouldn't contain a separate
	# /dts-v1/ tag and its node should be unique, i.e. the SP name.
	set(DTS_TAG "")
	set(DTS_NODE "${EXPORT_SP_NAME}")
	configure_file(${EXPORT_DTS_IN} ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dtsi @ONLY NEWLINE_STYLE UNIX)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dtsi DESTINATION ${TS_ENV}/manifest)

	# The .dts file is a standalone structure, thus it should have the /dts-v1/ tag and it
	# starts with the root node.
	set(DTS_TAG "/dts-v1/;")
	set(DTS_NODE "/")
	configure_file(${EXPORT_DTS_IN} ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dts @ONLY NEWLINE_STYLE UNIX)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_UUID}.dts DESTINATION ${TS_ENV}/manifest)

	if (DEFINED EXPORT_JSON_IN)
		configure_file(${EXPORT_JSON_IN} ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_NAME}.json @ONLY NEWLINE_STYLE UNIX)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_SP_NAME}.json DESTINATION ${TS_ENV}/json)
	endif()
endfunction()
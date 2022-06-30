#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
macro(generate_uuid_formats uuid)
	#Create a list of byte
	string(REGEX MATCHALL "([A-Za-z0-9][A-Za-z0-9])" SEPARATED_HEX "${uuid}")
	list(JOIN SEPARATED_HEX ", 0x" UUID_BYTES )
	#Generate the uuid_byte string
	#{ 0x01, 0x10, 0x9c, 0xf8, 0xe5, 0xca, 0x44, 0x6f,
	# 0x9b, 0x55, 0xf3, 0xcd, 0xc6, 0x51, 0x10, 0xc8, }

	string(PREPEND UUID_BYTES "{0x")
	string(APPEND UUID_BYTES "}")

	#Split the list of bytes in to the struct fields
	list(SUBLIST SEPARATED_HEX 0 4 uuid_timeLow)
	list(SUBLIST SEPARATED_HEX 4 2 uuid_timeMid)
	list(SUBLIST SEPARATED_HEX 6 2 uuid_timeHiAndVersion)
	list(SUBLIST SEPARATED_HEX 8 8 uuid_clockSeqAndNode)

	#Combine the bytes in the fields
	list(JOIN uuid_timeLow "" uuid_timeLow )
	string(PREPEND uuid_timeLow "0x")

	list(JOIN uuid_timeMid "" uuid_timeMid )
	string(PREPEND uuid_timeMid " 0x")

	list(JOIN uuid_timeHiAndVersion "" uuid_timeHiAndVersion )
	string(PREPEND uuid_timeHiAndVersion " 0x")

	list(JOIN uuid_clockSeqAndNode ", 0x" uuid_clockSeqAndNode )
	string(PREPEND uuid_clockSeqAndNode " 0x")

	#Combine the different fields into one uuid_struct string
	#{ 0x01109cf8, 0xe5ca, 0x446f, \
	#{ 0x9b, 0x55, 0xf3, 0xcd, 0xc6, 0x51, 0x10, 0xc8 } }

	string(CONCAT UUID_STRUCT "{" ${uuid_timeLow} "," ${uuid_timeMid}
		"," ${uuid_timeHiAndVersion} ", {" ${uuid_clockSeqAndNode} "}}")

	# Swith endianess
	list(SUBLIST SEPARATED_HEX 0 4 hex1)
	list(SUBLIST SEPARATED_HEX 4 4 hex2)
	list(SUBLIST SEPARATED_HEX 8 4 hex3)
	list(SUBLIST SEPARATED_HEX 12 4 hex4)

	list(REVERSE hex1)
	list(REVERSE hex2)
	list(REVERSE hex3)
	list(REVERSE hex4)
	string(CONCAT UUID_LE " 0x" ${hex1} " 0x" ${hex2} " 0x" ${hex3}
		" 0x" ${hex4})

endmacro()

#[===[.rst:
.. cmake:command:: set_target_uuids

.. code:: cmake

set_target_uuids(
	SP_UUID <uuid>
	SP_NAME <name>
	)

INPUTS:

``SP_UUID``
The UUID of the SP as a string.

``SP_NAME``
The name of the SP.

#]===]

function (set_target_uuids)
	set(options)
	set(oneValueArgs SP_UUID SP_NAME)
	set(multiValueArgs)
	cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}"
		"${multiValueArgs}" ${ARGN} )

	if(NOT DEFINED TARGET_SP_UUID)
		message(FATAL_ERROR "set_target_uuids: mandatory parameter SP_UUID not defined!")
	endif()
	if(NOT DEFINED TARGET_SP_NAME)
		message(FATAL_ERROR "set_target_uuids: mandatory parameter SP_NAME not defined!")
	endif()

	generate_uuid_formats(${TARGET_SP_UUID})
	target_compile_definitions(${TARGET_SP_NAME}
		PRIVATE OPTEE_SP_UUID=${UUID_STRUCT}
		PRIVATE OPTEE_SP_UUID_BYTES=${UUID_BYTES}
	)
	set(SP_UUID_LE ${UUID_LE} PARENT_SCOPE)
endfunction()

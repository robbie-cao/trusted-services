#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "attestation"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/fdt"
		"components/common/tlv"
		"components/common/trace"
		"components/common/utils"
		"components/common/endian"
		"components/common/uuid"
		"components/config/ramstore"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint"
		"components/rpc/ffarpc/caller/sp"
		"components/rpc/common/caller"
		"components/rpc/common/interface"
		"components/service/common/include"
		"components/service/common/client"
		"components/service/common/provider"
		"components/service/locator"
		"components/service/locator/interface"
		"components/service/locator/sp"
		"components/service/locator/sp/ffa"
		"components/service/attestation/include"
		"components/service/attestation/claims"
		"components/service/attestation/claims/sources/boot_seed_generator"
		"components/service/attestation/claims/sources/null_lifecycle"
		"components/service/attestation/claims/sources/instance_id"
		"components/service/attestation/claims/sources/implementation_id"
		"components/service/attestation/claims/sources/event_log"
		"components/service/attestation/claims/sources/event_log/mock"
		"components/service/attestation/reporter/local"
		"components/service/attestation/reporter/eat"
		"components/service/attestation/key_mngr/local"
		"components/service/attestation/provider"
		"components/service/attestation/provider/serializer/packed-c"
		"components/service/crypto/include"
		"components/service/crypto/client/psa"
		"protocols/rpc/common/packed-c"
)

target_sources(attestation PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/attestation_sp.c
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Get libc include dir
get_property(LIBC_INCLUDE_PATH TARGET c PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

# Qcbor
set (QCBOR_EXTERNAL_INCLUDE_PATHS ${LIBC_INCLUDE_PATH})
include(${TS_ROOT}/external/qcbor/qcbor.cmake)
target_link_libraries(attestation PRIVATE qcbor)

# t_cose
set (TCOSE_EXTERNAL_INCLUDE_PATHS ${LIBC_INCLUDE_PATH})
include(${TS_ROOT}/external/t_cose/t_cose.cmake)
target_link_libraries(attestation PRIVATE t_cose)

#################################################################

target_include_directories(attestation PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

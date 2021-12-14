#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)

# Set default platform.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the crypto deployment for opteesp
#
#  Builds the Crypto service provider for running in an SEL0 secure partition
#  hosted by OPTEE in the role of SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(crypto-sp)
target_include_directories(crypto-sp PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID "d9df52d5-16a2-4bb2-9aa4-d26d3b84e8c0")
set(SP_HEAP_SIZE "490 * 1024" CACHE STRING "SP heap size in bytes")
set(TRACE_PREFIX "CRYPTO" CACHE STRING "Trace prefix")

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the opteesp environment.
#
#-------------------------------------------------------------------------------
add_components(TARGET "crypto-sp"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/fdt"
		"components/common/tlv"
		"components/common/trace"
		"components/common/utils"
		"components/config/ramstore"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint"
		"components/rpc/ffarpc/caller/sp"
		"components/rpc/common/caller"
		"components/rpc/common/interface"
		"components/service/common/include"
		"components/service/common/client"
		"components/service/common/serializer/protobuf"
		"components/service/common/provider"
		"components/service/discovery/provider"
		"components/service/discovery/provider/serializer/packed-c"
		"components/service/crypto/provider"
		"components/service/crypto/provider/serializer/protobuf"
		"components/service/crypto/provider/serializer/packed-c"
		"components/service/crypto/provider/extension/hash"
		"components/service/crypto/provider/extension/hash/serializer/packed-c"
		"components/service/crypto/provider/extension/cipher"
		"components/service/crypto/provider/extension/cipher/serializer/packed-c"
		"components/service/crypto/provider/extension/key_derivation"
		"components/service/crypto/provider/extension/key_derivation/serializer/packed-c"
		"components/service/crypto/provider/extension/mac"
		"components/service/crypto/provider/extension/mac/serializer/packed-c"
		"components/service/crypto/factory/full"
		"components/service/crypto/backend/mbedcrypto"
		"components/service/crypto/backend/mbedcrypto/trng_adapter/platform"
		"components/service/secure_storage/include"
		"components/service/secure_storage/frontend/psa/its"
		"components/service/secure_storage/backend/secure_storage_client"
		"components/service/secure_storage/backend/null_store"
		"components/service/secure_storage/factory/sp/rot_store"
		"protocols/rpc/common/packed-c"
		"protocols/service/secure_storage/packed-c"
		"protocols/service/crypto/protobuf"
		"environments/opteesp"
)

target_sources(crypto-sp PRIVATE
	crypto_sp.c
)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET "crypto-sp")

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Get libc include dir
get_property(LIBC_INCLUDE_PATH TARGET stdlib::c PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

# Nanopb
list(APPEND NANOPB_EXTERNAL_INCLUDE_PATHS ${LIBC_INCLUDE_PATH})
include(../../../external/nanopb/nanopb.cmake)
target_link_libraries(crypto-sp PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "crypto-sp" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbed TLS provides libmbedcrypto
list(APPEND MBEDTLS_EXTRA_INCLUDES ${LIBC_INCLUDE_PATH})
include(../../../external/MbedTLS/MbedTLS.cmake)
target_link_libraries(crypto-sp PRIVATE mbedcrypto)
target_link_libraries(mbedcrypto INTERFACE stdlib::c)


#################################################################

target_compile_definitions(crypto-sp PRIVATE
	ARM64=1
)

target_include_directories(crypto-sp PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/crypto/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(crypto-sp PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		-std=c99
	)

	# Options for GCC that control linking
	target_link_options(crypto-sp PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(crypto-sp PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET crypto-sp NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
#TODO: api headers

install(TARGETS crypto-sp
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

get_property(_PROTO_FILES TARGET crypto-sp PROPERTY PROTOBUF_FILES)
install(FILES ${_PROTO_FILES} DESTINATION ${TS_ENV}/lib/protobuf)

include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake)
export_sp(
	SP_UUID ${SP_UUID}
	SP_NAME "crypto"
	MK_IN ${TS_ROOT}/environments/opteesp/sp.mk.in
	DTS_IN ${CMAKE_CURRENT_LIST_DIR}/default_crypto.dts.in
	JSON_IN ${TS_ROOT}/environments/opteesp/sp_pkg.json.in
)

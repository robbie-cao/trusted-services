#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "crypto"
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
)

target_sources(crypto PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/crypto_sp.c
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Get libc include dir
get_property(LIBC_INCLUDE_PATH TARGET stdlib::c PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

# Nanopb
list(APPEND NANOPB_EXTERNAL_INCLUDE_PATHS ${LIBC_INCLUDE_PATH})
include(../../../external/nanopb/nanopb.cmake)
target_link_libraries(crypto PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "crypto" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbed TLS provides libmbedcrypto
list(APPEND MBEDTLS_EXTRA_INCLUDES ${LIBC_INCLUDE_PATH})
include(../../../external/MbedTLS/MbedTLS.cmake)
target_link_libraries(crypto PRIVATE mbedcrypto)
target_link_libraries(mbedcrypto INTERFACE stdlib::c)

#################################################################

target_include_directories(crypto PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

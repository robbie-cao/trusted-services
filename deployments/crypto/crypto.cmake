#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "crypto"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/tlv"
		"components/rpc/common/interface"
		"components/service/common/include"
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
		"components/service/crypto/provider/extension/aead"
		"components/service/crypto/provider/extension/aead/serializer/packed-c"
		"components/service/crypto/factory/full"
		"components/service/crypto/backend/mbedcrypto"
		"protocols/rpc/common/packed-c"
		"protocols/service/crypto/protobuf"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(crypto PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "crypto" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbed TLS provides libmbedcrypto
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(crypto PRIVATE mbedcrypto)

#################################################################

target_include_directories(crypto PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

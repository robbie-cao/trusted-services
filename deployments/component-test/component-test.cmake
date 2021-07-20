#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'component-test' for
#  different environments.  Used for running standalone component tests
#  contained within a single executable.
#-------------------------------------------------------------------------------
add_components(
	TARGET "component-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/app/ts-demo"
		"components/app/ts-demo/test"
		"components/common/uuid"
		"components/common/uuid/test"
		"components/common/tlv"
		"components/common/tlv/test"
		"components/common/endian"
		"components/common/endian/test"
		"components/config/ramstore"
		"components/config/ramstore/test"
		"components/rpc/common/caller"
		"components/rpc/common/interface"
		"components/rpc/common/demux"
		"components/rpc/common/test/protocol"
		"components/rpc/direct"
		"components/rpc/dummy"
		"components/service/common/include"
		"components/service/common/serializer/protobuf"
		"components/service/common/provider"
		"components/service/common/provider/test"
		"components/service/locator"
		"components/service/locator/interface"
		"components/service/locator/test"
		"components/service/locator/standalone"
		"components/service/locator/standalone/services/crypto"
		"components/service/locator/standalone/services/internal-trusted-storage"
		"components/service/locator/standalone/services/protected-storage"
		"components/service/locator/standalone/services/test-runner"
		"components/service/locator/standalone/services/attestation"
		"components/service/attestation/include"
		"components/service/attestation/claims"
		"components/service/attestation/claims/sources/boot_seed_generator"
		"components/service/attestation/claims/sources/null_lifecycle"
		"components/service/attestation/claims/sources/instance_id"
		"components/service/attestation/claims/sources/event_log"
		"components/service/attestation/claims/sources/event_log/mock"
		"components/service/attestation/claims/sources/event_log/test"
		"components/service/attestation/reporter/local"
		"components/service/attestation/reporter/eat"
		"components/service/attestation/reporter/dump/raw"
		"components/service/attestation/key_mngr/local"
		"components/service/attestation/provider"
		"components/service/attestation/provider/serializer/packed-c"
		"components/service/attestation/client/psa"
		"components/service/attestation/client/provision"
		"components/service/attestation/test/component"
		"components/service/attestation/test/service"
		"components/service/crypto/client/cpp"
		"components/service/crypto/client/cpp/protobuf"
		"components/service/crypto/client/cpp/packed-c"
		"components/service/crypto/client/test"
		"components/service/crypto/client/test/standalone"
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
		"components/service/crypto/provider/test"
		"components/service/crypto/backend/mbedcrypto"
		"components/service/crypto/factory/full"
		"components/service/crypto/test/unit"
		"components/service/crypto/test/service"
		"components/service/crypto/test/service/protobuf"
		"components/service/crypto/test/service/packed-c"
		"components/service/crypto/test/protocol"
		"components/service/secure_storage/include"
		"components/service/secure_storage/frontend/psa/its"
		"components/service/secure_storage/frontend/psa/its/test"
		"components/service/secure_storage/frontend/psa/ps"
		"components/service/secure_storage/frontend/psa/ps/test"
		"components/service/secure_storage/frontend/secure_storage_provider"
		"components/service/secure_storage/backend/secure_storage_client"
		"components/service/secure_storage/backend/secure_storage_client/test"
		"components/service/secure_storage/backend/null_store"
		"components/service/secure_storage/backend/mock_store"
		"components/service/secure_storage/backend/mock_store/test"
		"components/service/secure_storage/backend/secure_flash_store"
		"components/service/secure_storage/backend/secure_flash_store/test"
		"components/service/secure_storage/backend/secure_flash_store/flash_fs"
		"components/service/secure_storage/backend/secure_flash_store/flash"
		"components/service/test_runner/provider"
		"components/service/test_runner/provider/serializer/packed-c"
		"components/service/test_runner/provider/backend/null"
		"protocols/rpc/common/protobuf"
		"protocols/rpc/common/packed-c"
		"protocols/service/crypto/packed-c"
		"protocols/service/crypto/protobuf"
		"protocols/service/secure_storage/packed-c"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(component-test PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "component-test" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbed TLS provides libmbedcrypto
include(${TS_ROOT}/external/MbedTLS/MbedTLS.cmake)
target_link_libraries(component-test PRIVATE mbedcrypto)

# Qcbor
include(${TS_ROOT}/external/qcbor/qcbor.cmake)
target_link_libraries(component-test PRIVATE qcbor)

# t_cose
include(${TS_ROOT}/external/t_cose/t_cose.cmake)
target_link_libraries(component-test PRIVATE t_cose)

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS component-test DESTINATION ${TS_ENV}/bin)

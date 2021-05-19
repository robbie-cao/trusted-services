#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'ts-service-test' for
#  different environments.  Used for running end-to-end service-level tests
#  where test cases excerise trusted service client interfaces.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Use libts for locating and accessing services. An appropriate version of
#  libts will be imported for the enviroment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(ts-service-test PRIVATE libts)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "ts-service-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/tlv"
		"components/service/common/include"
		"components/service/crypto/include"
		"components/service/crypto/test/service"
		"components/service/crypto/test/service/protobuf"
		"components/service/crypto/test/service/packed-c"
		"components/service/crypto/test/service/psa_crypto_api"
		"components/service/crypto/client/psa"
		"components/service/crypto/client/cpp"
		"components/service/crypto/client/cpp/protobuf"
		"components/service/crypto/client/cpp/packed-c"
		"components/service/common/serializer/protobuf"
		"components/service/attestation/include"
		"components/service/attestation/client/psa"
		"components/service/attestation/client/provision"
		"components/service/attestation/test/service"
		"protocols/service/crypto/protobuf"
		"protocols/service/crypto/packed-c"
		"components/service/secure_storage/include"
		"components/service/secure_storage/test/service"
		"components/service/secure_storage/frontend/psa/its"
		"components/service/secure_storage/frontend/psa/its/test"
		"components/service/secure_storage/frontend/psa/ps"
		"components/service/secure_storage/frontend/psa/ps/test"
		"components/service/secure_storage/backend/secure_storage_client"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(ts-service-test PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "ts-service-test" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS ts-service-test RUNTIME DESTINATION ${TS_ENV}/bin)

#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the se-proxy deployment for opteesp
#
#  Builds proxy service providers that communicate with a separate secure element
#  that hosts a set of service endpoints.  This deployment is for running in an
#  SEL0 secure partition hosted by OPTEE in the role of SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(se-proxy)
target_include_directories(se-proxy PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID "46bb39d1-b4d9-45b5-88ff-040027dab249")

#-------------------------------------------------------------------------------
#  Include SP DEV KIT interface
#
#-------------------------------------------------------------------------------
set(SP_DEV_KIT_INC_DIR ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH "${TS_ROOT}/external/Spdevkit")
find_package(Spdevkit REQUIRED)
sp_dev_kit_configure_linking(TARGET se-proxy DEFINES ARM64=1)
target_link_libraries(se-proxy PRIVATE ${SP_DEV_KIT_LIBRARIES})

#-------------------------------------------------------------------------------
#  Default deployment specific configuration
#
#-------------------------------------------------------------------------------
set(TS_NO_FLOAT_HW ON)

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the opteesp environment.
#
#-------------------------------------------------------------------------------
add_components(TARGET "se-proxy"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"protocols/rpc/common/packed-c"
		"protocols/service/secure_storage/packed-c"
		"protocols/service/crypto/protobuf"
		"environments/opteesp"
		"components/common/tlv"
		"components/config/ramstore"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint"
		"components/rpc/common/interface"
		"components/rpc/common/demux"
		"components/service/common/include"
		"components/service/common/serializer/protobuf"
		"components/service/common/provider"
		"components/service/crypto/provider"
		"components/service/crypto/provider/serializer/protobuf"
		"components/service/crypto/provider/serializer/packed-c"
		"components/service/secure_storage/include"
		"components/service/secure_storage/frontend/secure_storage_provider"
		"components/service/attestation/include"
		"components/service/attestation/provider"
		"components/service/attestation/provider/serializer/packed-c"

		# Components that won't be in a proxy
		"components/common/endian"
		"components/service/attestation/claims"
		"components/service/attestation/claims/sources/boot_seed_generator"
		"components/service/attestation/claims/sources/null_lifecycle"
		"components/service/attestation/claims/sources/instance_id"
		"components/service/attestation/claims/sources/event_log"
		"components/service/attestation/claims/sources/event_log/mock"
		"components/service/attestation/reporter/psa"
		"components/service/attestation/key_mngr"
		"components/service/secure_storage/frontend/psa/its"
		"components/service/secure_storage/backend/secure_flash_store"
		"components/service/secure_storage/backend/secure_flash_store/flash_fs"
		"components/service/secure_storage/backend/secure_flash_store/flash"
		"components/service/crypto/backend/mbedcrypto"
		"components/service/crypto/backend/mbedcrypto/trng_adapter/platform"
)

target_sources(se-proxy PRIVATE
	se_proxy_sp.c
	service_proxy_factory.c
)

#-------------------------------------------------------------------------------
#  Use the selected platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
# temporarily force platform - with this change, the build interface to
# an external builder such as a Yocto recipe is unchanged.  Should remove
# once the build interface is published.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Overridden" FORCE)

add_platform(TARGET "se-proxy")

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
list(APPEND NANOPB_EXTERNAL_INCLUDE_PATHS ${SP_DEV_KIT_INCLUDE_DIR})
include(../../../external/nanopb/nanopb.cmake)
target_link_libraries(se-proxy PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "se-proxy" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

# Mbed TLS provides libmbedcrypto
include(../../../external/MbedTLS/MbedTLS.cmake)
target_link_libraries(se-proxy PRIVATE mbedcrypto)

# Qcbor
include(${TS_ROOT}/external/qcbor/qcbor.cmake)
target_link_libraries(se-proxy PRIVATE qcbor)

# t_cose
include(${TS_ROOT}/external/t_cose/t_cose.cmake)
target_link_libraries(se-proxy PRIVATE t_cose)

#################################################################

target_compile_definitions(se-proxy PRIVATE
	ARM64=1
)

target_include_directories(se-proxy PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/se-proxy/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(se-proxy PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		-std=c99
	)

	# Options for GCC that control linking
	target_link_options(se-proxy PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(se-proxy PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET se-proxy NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
#TODO: api headers

install(TARGETS se-proxy
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

get_property(_PROTO_FILES TARGET se-proxy PROPERTY PROTOBUF_FILES)
install(FILES ${_PROTO_FILES} DESTINATION ${TS_ENV}/lib/protobuf)


set(EXPORT_SP_NAME "se-proxy")
set(EXPORT_SP_UUID ${SP_UUID})
include(${TS_ROOT}/environments/opteesp/ExportSp.cmake)

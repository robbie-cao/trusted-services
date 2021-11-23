#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the smm-gateway deployment for opteesp
#
#  Provides a service provider for UEFI SMM services that are delegated to
#  other trusted service providers.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(smm-gateway)
target_include_directories(smm-gateway PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID "ed32d533-99e6-4209-9cc0-2d72cdd998a7")


# Include SP DEV KIT interface
set(SP_DEV_KIT_INC_DIR ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH "${TS_ROOT}/external/Spdevkit")
find_package(Spdevkit REQUIRED)
sp_dev_kit_configure_linking(TARGET smm-gateway DEFINES ARM64=1)
target_link_libraries(smm-gateway PRIVATE ${SP_DEV_KIT_LIBRARIES})

#-------------------------------------------------------------------------------
#  Components that are specific to deployment in the opteesp environment.
#
#-------------------------------------------------------------------------------
add_components(TARGET "smm-gateway"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
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
		"components/service/smm_variable/backend"
		"components/service/smm_variable/provider"
		"components/service/secure_storage/include"
		"components/service/secure_storage/backend/secure_storage_client"
		"components/service/secure_storage/backend/mock_store"
		"protocols/rpc/common/packed-c"
		"protocols/service/secure_storage/packed-c"
		"environments/opteesp"
)

target_sources(smm-gateway PRIVATE
	smm_gateway_sp.c
	../smm_gateway.c
)

#-------------------------------------------------------------------------------
#  Use the selected platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
# temporarily force platform - with this change, the build interface to
# an external builder such as a Yocto recipe is unchanged.  Should remove
# once the build interface is published.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Overridden" FORCE)

add_platform(TARGET "smm-gateway")

#################################################################

target_compile_definitions(smm-gateway PRIVATE
	ARM64=1
)

target_include_directories(smm-gateway PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/smm-gateway/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(smm-gateway PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		-std=c99
	)

	# Options for GCC that control linking
	target_link_options(smm-gateway PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(smm-gateway PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET smm-gateway NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
#TODO: api headers

install(TARGETS smm-gateway
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

get_property(_PROTO_FILES TARGET smm-gateway PROPERTY PROTOBUF_FILES)
install(FILES ${_PROTO_FILES} DESTINATION ${TS_ENV}/lib/protobuf)


set(EXPORT_SP_NAME "smm-gateway")
set(EXPORT_SP_UUID ${SP_UUID})
include(${TS_ROOT}/environments/opteesp/ExportSp.cmake)

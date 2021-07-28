#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the sfs-demo deployment for opteesp
#
#  Used for building a demo sp that acts as a client of the secure storage
#  service, deployed in another sp.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(sfs-demo)
set(SP_UUID "01109cf8-e5ca-446f-9b55-f3cdc65110c8")


# Include SP DEV KIT interface
set(SP_DEV_KIT_INC_DIR ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH "${TS_ROOT}/external/Spdevkit")
find_package(Spdevkit REQUIRED)
sp_dev_kit_configure_linking(TARGET sfs-demo DEFINES ARM64=1)
target_link_libraries(sfs-demo ${SP_DEV_KIT_LIBRARIES})


add_components(TARGET "sfs-demo"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/messaging/ffa/libsp
		components/rpc/common/interface
		components/rpc/common/caller
		components/rpc/ffarpc/caller/sp
		components/service/common/include
		components/service/common/client
		components/service/secure_storage/include
		components/service/secure_storage/frontend/psa/its
		components/service/secure_storage/backend/secure_storage_client
		protocols/rpc/common/packed-c
		protocols/service/secure_storage/packed-c
		environments/opteesp
)

target_sources(sfs-demo PRIVATE
	sp.c
)

target_compile_definitions(sfs-demo PRIVATE
	ARM64=1
)

target_include_directories(sfs-demo PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/sfs-demo/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(sfs-demo PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		-std=c99
	)

	# Options for GCC that control linking
	target_link_options(sfs-demo PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(sfs-demo PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET sfs-demo NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
#TODO: API header, protobuf files?
install(TARGETS sfs-demo DESTINATION ${TS_ENV}/bin)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

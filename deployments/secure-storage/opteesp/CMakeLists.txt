#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the secure-storage deployment for opteesp
#
#  Builds the secure storage service provider for running in an SEL0 secure
#  partition hosted by OPTEE in the role of SPM.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(secure-storage)
target_include_directories(secure-storage PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID "dc1eef48-b17a-4ccf-ac8b-dfcff7711b14")


# Include SP DEV KIT interface
set(SP_DEV_KIT_INC_DIR ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH "${TS_ROOT}/external/Spdevkit")
find_package(Spdevkit COMPONENTS SP_HEADER interface)

sp_dev_kit_configure_linking(TARGET secure-storage DEFINES ARM64=1)
target_link_libraries(secure-storage ${SP_DEV_KIT_LIBRARIES})

add_components(TARGET "secure-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/messaging/ffa/libsp
		components/rpc/ffarpc/endpoint
		components/rpc/common/interface
		components/service/common/provider
		components/service/secure_storage/provider/secure_flash_store
		components/service/secure_storage/provider/secure_flash_store/flash_fs
		components/service/secure_storage/provider/secure_flash_store/flash
		protocols/rpc/common/packed-c
		protocols/service/secure_storage/packed-c
		environments/opteesp
)

target_sources(secure-storage PRIVATE
	sp.c
)

target_compile_definitions(secure-storage PRIVATE
	ARM64=1
)

target_include_directories(secure-storage PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/secure-storage/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(secure-storage PRIVATE
		-fdiagnostics-show-option
		-fpic
		-gdwarf-2
		-mstrict-align
		-O0
		-std=gnu99
	)

	# Options for GCC that control linking
	target_link_options(secure-storage PRIVATE
		-e __sp_entry
		-fno-lto
		-nostdlib
		-pie
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(secure-storage PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET secure-storage NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS secure-storage
			PUBLIC_HEADER DESTINATION include
			RUNTIME DESTINATION bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION bin)

set(EXPORT_SP_NAME "secure-storage")
set(EXPORT_SP_UUID ${SP_UUID})
include(${TS_ROOT}/environments/opteesp/ExportSp.cmake)

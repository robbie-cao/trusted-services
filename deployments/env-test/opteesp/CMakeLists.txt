#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the env-test deployment for opteesp
#
#  Builds the test_runner service provider for running in an SEL0 secure partition
#  hosted by OPTEE in the role of SPM.  Environment tests are added and CppUnit
#  test cases.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C CXX ASM)
add_executable(env_test)
target_include_directories(env_test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID "33c75baf-ac6a-4fe4-8ac7-e9909bee2d17")


# Include SP DEV KIT interface
set(SP_DEV_KIT_INC_DIR ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH "${TS_ROOT}/external/Spdevkit")
find_package(Spdevkit REQUIRED)
sp_dev_kit_configure_linking(TARGET env_test DEFINES ARM64=1)
target_link_libraries(env_test PRIVATE ${SP_DEV_KIT_LIBRARIES})

#-------------------------------------------------------------------------------
#  Components that are env_testecific to deployment in the opteesp
#  environment.
#-------------------------------------------------------------------------------
add_components(TARGET "env_test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/tlv"
		"components/config/ramstore"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint"
		"components/rpc/common/interface"
		"components/service/common"
		"components/service/common/provider"
		"components/service/test_runner/provider"
		"components/service/test_runner/provider/serializer/packed-c"
		"components/service/test_runner/provider/backend/null"
		"protocols/rpc/common/packed-c"
		"environments/opteesp"
)

target_sources(env_test PRIVATE
	env_test.c
	env_test_config.c
)

#-------------------------------------------------------------------------------
#  Use the selected platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
# temporarily force platform - remove when external builder updated
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Overridden" FORCE)

add_platform(TARGET "env_test")

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

if(CMAKE_CROSSCOMPILING)
	target_link_libraries(env_test PRIVATE stdc++ gcc m)
endif()

#################################################################

target_compile_definitions(env_test PRIVATE
	ARM64=1
)

target_include_directories(env_test PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/env-test/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(env_test PRIVATE
		-fdiagnostics-show-option
		-fpic
		-gdwarf-2
		-mstrict-align
		-O0
		$<$<COMPILE_LANGUAGE:C>:-std=gnu99>
		$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	)

	# Options for GCC that control linking
	target_link_options(env_test PRIVATE
		-e __sp_entry
		-fno-lto
		-nostdlib
		-pie
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(env_test PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET env_test NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
#TODO: api headers
install(TARGETS env_test
			PUBLIC_HEADER DESTINATION include
			RUNTIME DESTINATION bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION bin)

set(EXPORT_SP_NAME "env-test")
set(EXPORT_SP_UUID ${SP_UUID})
include(${TS_ROOT}/environments/opteesp/ExportSp.cmake)

#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)

# Set default platform.
set(TS_PLATFORM "arm/fvp/fvp_base_revc-2xaemv8a" CACHE STRING "Target platform location.")
include(../../deployment.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the env-test deployment for opteesp
#
#  Builds the test_runner service provider for running in an SEL0 secure partition
#  hosted by OPTEE in the role of SPM.  Environment tests are added and CppUnit
#  test cases.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/environments/opteesp/env.cmake)
project(trusted-services LANGUAGES C ASM)
add_executable(env-test)
target_include_directories(env-test PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")
set(SP_UUID "33c75baf-ac6a-4fe4-8ac7-e9909bee2d17")
set(TRACE_PREFIX "ENVTEST" CACHE STRING "Trace prefix")

#-------------------------------------------------------------------------------
#  Components that are env-test specific to deployment in the opteesp
#  environment.
#-------------------------------------------------------------------------------
add_components(TARGET "env-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/trace"
		"components/common/utils"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint/sp"
		"components/config/test/sp"
		"environments/opteesp"
)

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  env-test
#
#-------------------------------------------------------------------------------
include(../env_test.cmake REQUIRED)

#-------------------------------------------------------------------------------
#  Deployment specific source files
#-------------------------------------------------------------------------------
target_sources(env-test PRIVATE
	env_test.c
	env_test_tests.c
)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET env-test)

if(CMAKE_CROSSCOMPILING)
	target_link_libraries(env-test PRIVATE stdc++ gcc m)
endif()

#################################################################

target_compile_definitions(env-test PRIVATE
	ARM64=1
)

target_include_directories(env-test PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/env-test/opteesp
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(env-test PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		$<$<COMPILE_LANGUAGE:C>:-std=c99>
		$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	)

	# Options for GCC that control linking
	target_link_options(env-test PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(env-test PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET env-test NAME "${SP_UUID}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
#TODO: api headers

install(TARGETS env-test
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)

get_property(_PROTO_FILES TARGET env-test PROPERTY PROTOBUF_FILES)
install(FILES ${_PROTO_FILES} DESTINATION ${TS_ENV}/lib/protobuf)


set(EXPORT_SP_NAME "env-test")
set(EXPORT_SP_UUID ${SP_UUID})
include(${TS_ROOT}/environments/opteesp/ExportSp.cmake)

#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
#  The CMakeLists.txt for building the spm-test sp deployment for opteesp
#
#  Used for building the SPs used in the  spm test. The SP can be build twice
#  , to be able to test inter SPs communication. This is done by passing the
#  -DSP_NUMBER=1 parameter.
#-------------------------------------------------------------------------------
target_include_directories(spm-test${SP_NUMBER} PRIVATE "${TOP_LEVEL_INCLUDE_DIRS}")

include(${TS_ROOT}/tools/cmake/common/TargetCompileDefinitions.cmake)
set_target_uuids(
	SP_UUID ${SP_UUID_CANON}
	SP_NAME "spm-test${SP_NUMBER}"
)
set(SP_HEAP_SIZE "32 * 1024" CACHE STRING "SP heap size in bytes")
set(TRACE_PREFIX "SPM-TEST${SP_NUMBER}" CACHE STRING "Trace prefix")

#-------------------------------------------------------------------------------
#  Extend with components that are common across all deployments of
#  spm-test
#
#-------------------------------------------------------------------------------
target_include_directories(spm-test${SP_NUMBER} PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

#-------------------------------------------------------------------------------
#  Set target platform to provide drivers needed by the deployment
#
#-------------------------------------------------------------------------------
add_platform(TARGET spm-test${SP_NUMBER})

#################################################################

target_compile_definitions(spm-test${SP_NUMBER} PRIVATE
	ARM64=1
)

target_include_directories(spm-test${SP_NUMBER} PRIVATE
	${TS_ROOT}/components/service/spm_test
)

#-------------------------------------------------------------------------------
#  Deployment specific source files
#-------------------------------------------------------------------------------
target_sources(spm-test${SP_NUMBER} PRIVATE
	${TS_ROOT}/components/service/spm_test/sp.c
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(spm-test${SP_NUMBER} PRIVATE
		-fdiagnostics-show-option
		-gdwarf-2
		-mstrict-align
		-O0
		$<$<COMPILE_LANGUAGE:C>:-std=c99>
		$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	)

	# Options for GCC that control linking
	target_link_options(spm-test${SP_NUMBER} PRIVATE
		-zmax-page-size=4096
	)
	# Options directly for LD, these are not understood by GCC
	target_link_options(spm-test${SP_NUMBER} PRIVATE
		-Wl,--as-needed
		-Wl,--sort-section=alignment
		# -Wl,--dynamic-list ${CMAKE_CURRENT_LIST_DIR}/dyn_list
	)
endif()

compiler_generate_stripped_elf(TARGET spm-test${SP_NUMBER} NAME "${SP_UUID_CANON}.stripped.elf" RES STRIPPED_ELF)

######################################## install
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()

set_target_properties(spm-test${SP_NUMBER} PROPERTIES OUTPUT_NAME "spm-test${SP_NUMBER}-${SP_UUID_CANON}.elf" )

install(TARGETS spm-test${SP_NUMBER}
			PUBLIC_HEADER DESTINATION ${TS_ENV}/include
			RUNTIME DESTINATION ${TS_ENV}/bin
		)
install(FILES ${STRIPPED_ELF} DESTINATION ${TS_ENV}/bin)


include(${TS_ROOT}/tools/cmake/common/ExportSp.cmake)
export_sp(
	SP_UUID_CANON ${SP_UUID_CANON}
	SP_UUID_LE ${SP_UUID_LE}
	SP_NAME "spm-test${SP_NUMBER}"
	MK_IN ${TS_ROOT}/environments/opteesp/sp.mk.in
	DTS_IN ${TS_ROOT}/deployments/spm-test${SP_NUMBER}/opteesp/default_spm_test${SP_NUMBER}.dts.in
	JSON_IN ${TS_ROOT}/environments/opteesp/sp_pkg.json.in
)

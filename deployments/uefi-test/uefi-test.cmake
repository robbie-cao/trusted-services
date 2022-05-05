#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  The base build file shared between deployments of 'uefi-test' for
#  different environments.  Used for running end-to-end service-level tests
#  against SMM service providers that implement UEFI services such as smm
#  variable.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#  Use libts for locating and accessing services. An appropriate version of
#  libts will be imported for the enviroment in which service tests are
#  deployed.
#-------------------------------------------------------------------------------
include(${TS_ROOT}/deployments/libts/libts-import.cmake)
target_link_libraries(uefi-test PRIVATE libts::ts)

#-------------------------------------------------------------------------------
#  Components that are common accross all deployments
#
#-------------------------------------------------------------------------------
add_components(
	TARGET "uefi-test"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/service/uefi/smm_variable/client/cpp"
		"components/service/uefi/smm_variable/test/service"
)

#-------------------------------------------------------------------------------
#  Components used from external projects
#
#-------------------------------------------------------------------------------

# Nanopb
include(${TS_ROOT}/external/nanopb/nanopb.cmake)
target_link_libraries(uefi-test PRIVATE nanopb::protobuf-nanopb-static)
protobuf_generate_all(TGT "uefi-test" NAMESPACE "protobuf" BASE_DIR "${TS_ROOT}/protocols")

#-------------------------------------------------------------------------------
#  Define install content.
#
#-------------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "location to install build output to." FORCE)
endif()
install(TARGETS uefi-test RUNTIME DESTINATION ${TS_ENV}/bin)

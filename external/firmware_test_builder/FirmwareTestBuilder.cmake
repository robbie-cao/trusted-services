#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(FetchContent)

set(FIRMWARE_TEST_BUILDER_URL "https://git.trustedfirmware.org/TS/trusted-services.git" CACHE STRING "firmware-test-builder repository URL")
set(FIRMWARE_TEST_BUILDER_REFSPEC "topics/fwtb" CACHE STRING "firmware-test-builder git refspec")

# Checking git
find_program(GIT_COMMAND "git")
if (NOT GIT_COMMAND)
	message(FATAL_ERROR "Please install git")
endif()

# Fetching firmware-test-builder
FetchContent_Declare(
	firmware_test_builder
	GIT_REPOSITORY ${FIRMWARE_TEST_BUILDER_URL}
	GIT_TAG ${FIRMWARE_TEST_BUILDER_REFSPEC}
	GIT_SHALLOW TRUE
)

FetchContent_GetProperties(firmware_test_builder)
if(NOT firmware_test_builder_POPULATED)
	message(STATUS "Fetching Firmware Test Builder")
	FetchContent_Populate(firmware_test_builder)
endif()

# Appending firmware-test-builder's CMake directory to CMake module path
list(APPEND CMAKE_MODULE_PATH ${firmware_test_builder_SOURCE_DIR}/cmake)
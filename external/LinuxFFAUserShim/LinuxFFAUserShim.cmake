#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Find Linux FF-A user space shim repo location.
# It contains a kernel module which exposes FF-A operations to user space using DebugFS.

# If a CMake variable exists, use it as is.
# If not, try to copy the value from the environment.
# If neither is present, try to download.
if(NOT DEFINED LINUX_FFA_USER_SHIM_DIR)
	if(DEFINED ENV{LINUX_FFA_USER_SHIM_DIR})
		set(LINUX_FFA_USER_SHIM_DIR $ENV{LINUX_FFA_USER_SHIM_DIR}
				CACHE STRING "Linux FF-A user space shim dir")
	else()
		set(LINUX_FFA_USER_SHIM_URL "https://git.gitlab.arm.com/linux-arm/linux-trusted-services.git"
				CACHE STRING "Linux FF-A user space shim repository URL")
		set(LINUX_FFA_USER_SHIM_REFSPEC "v3.0.0"
				CACHE STRING "Linux FF-A user space shim git refspec")

		find_program(GIT_COMMAND "git")
		if (NOT GIT_COMMAND)
			message(FATAL_ERROR "Please install git")
		endif()

		include(FetchContent)
		FetchContent_Declare(linux_ffa_user_shim
			GIT_REPOSITORY ${LINUX_FFA_USER_SHIM_URL}
			GIT_TAG ${LINUX_FFA_USER_SHIM_REFSPEC}
			GIT_SHALLOW TRUE
		)

		# FetchContent_GetProperties exports <name>_SOURCE_DIR and <name>_BINARY_DIR variables
		FetchContent_GetProperties(linux_ffa_user_shim)
		if(NOT linux_ffa_user_shim_POPULATED)
			message(STATUS "Fetching Linux FF-A user space shim")
			FetchContent_Populate(linux_ffa_user_shim)
		endif()

		set(LINUX_FFA_USER_SHIM_DIR ${linux_ffa_user_shim_SOURCE_DIR}
				CACHE STRING "Linux FF-A user space shim dir")
	endif()
endif()

find_path(LINUX_FFA_USER_SHIM_INCLUDE_DIR
	NAMES arm_ffa_user.h
	PATHS ${LINUX_FFA_USER_SHIM_DIR}
	NO_DEFAULT_PATH
	REQUIRED
	DOC "Linux FF-A user space shim include directory"
)

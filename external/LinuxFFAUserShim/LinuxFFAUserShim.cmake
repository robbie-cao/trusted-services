#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Find Linux FF-A user space shim repo location.
# It contains a kernel module which exposes FF-A operations to user space using DebugFS.


set(LINUX_FFA_USER_SHIM_URL "https://git.gitlab.arm.com/linux-arm/linux-trusted-services.git"
	CACHE STRING "Linux FF-A user space shim repository URL")
set(LINUX_FFA_USER_SHIM_REFSPEC "v4.0.0"
	CACHE STRING "Linux FF-A user space shim git refspec")

set(LINUX_FFA_USER_SHIM_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/linux_ffa_user_shim-src"
	CACHE PATH "Location of Linux driver source.")

if (DEFINED ENV{LINUX_FFA_USER_SHIM_SOURCE_DIR})
	set(LINUX_FFA_USER_SHIM_SOURCE_DIR $ENV{LINUX_FFA_USER_SHIM_SOURCE_DIR}
		CACHE PATH "Location of Linux driver source." FORCE)
endif()

set(GIT_OPTIONS
	GIT_REPOSITORY ${LINUX_FFA_USER_SHIM_URL}
	GIT_TAG ${LINUX_FFA_USER_SHIM_REFSPEC}
	GIT_SHALLOW FALSE
	)
	include(${TS_ROOT}/tools/cmake/common/LazyFetch.cmake REQUIRED)
	LazyFetch_MakeAvailable(
		DEP_NAME linux_ffa_user_shim
		FETCH_OPTIONS "${GIT_OPTIONS}"
		SOURCE_DIR ${LINUX_FFA_USER_SHIM_SOURCE_DIR}
	)

find_path(LINUX_FFA_USER_SHIM_INCLUDE_DIR
	NAMES arm_ffa_user.h
	PATHS ${LINUX_FFA_USER_SHIM_SOURCE_DIR}
	NO_DEFAULT_PATH
	REQUIRED
	DOC "Linux FF-A user space shim include directory"
)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
	"${LINUX_FFA_USER_SHIM_INCLUDE_DIR}/arm_ffa_user.h")

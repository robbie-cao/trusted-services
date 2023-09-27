#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# If the driver is already installed, try to find that
find_path(LINUX_FFA_TEE_DRIVER_INCLUDE_DIR
	NAMES arm_tstee.h
	DOC "Linux FF-A TEE driver include directory"
)

# If not found, download it
if(NOT LINUX_FFA_TEE_DRIVER_INCLUDE_DIR)
	set(LINUX_FFA_TEE_DRIVER_URL "https://git.gitlab.arm.com/linux-arm/linux-trusted-services.git"
		CACHE STRING "Linux FF-A TEE driver repository URL")

	# Note: the aim of this external component is to make the header file defining the IOCTL API
	#        available. Fetching a moving reference is ok as long as API compatibility is guaranteed.
	set(LINUX_FFA_TEE_DRIVER_REFSPEC "origin/tee-v2"
		CACHE STRING "Linux FF-A TEE driver git refspec")

	set(LINUX_FFA_TEE_DRIVER_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/linux_ffa_tee_driver-src"
		CACHE PATH "Location of Linux TEE driver source.")

	if (DEFINED ENV{LINUX_FFA_TEE_DRIVER_SOURCE_DIR})
		set(LINUX_FFA_TEE_DRIVER_SOURCE_DIR $ENV{LINUX_FFA_TEE_DRIVER_SOURCE_DIR}
			CACHE PATH "Location of Linux TEE driver source." FORCE)
	endif()

	set(GIT_OPTIONS
		GIT_REPOSITORY ${LINUX_FFA_TEE_DRIVER_URL}
		GIT_TAG ${LINUX_FFA_TEE_DRIVER_REFSPEC}
		GIT_SHALLOW TRUE
		)
		include(${TS_ROOT}/tools/cmake/common/LazyFetch.cmake REQUIRED)
		LazyFetch_MakeAvailable(
			DEP_NAME linux_ffa_tee_driver
			FETCH_OPTIONS "${GIT_OPTIONS}"
			SOURCE_DIR ${LINUX_FFA_TEE_DRIVER_SOURCE_DIR}
		)

	find_path(LINUX_FFA_TEE_DRIVER_INCLUDE_DIR
		NAMES arm_tstee.h
		PATHS ${LINUX_FFA_TEE_DRIVER_SOURCE_DIR}/uapi
		NO_DEFAULT_PATH
		REQUIRED
		DOC "Linux FF-A TEE driver include directory"
	)
endif()

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
	"${LINUX_FFA_TEE_DRIVER_INCLUDE_DIR}/arm_tstee.h")

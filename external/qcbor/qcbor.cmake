#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# QCBOR is a library for encoding and decoding CBOR objects, as per RFC8949
#-------------------------------------------------------------------------------

set(QCBOR_URL "https://github.com/laurencelundblade/QCBOR.git" CACHE STRING "qcbor repository URL")
set(QCBOR_REFSPEC "v1.0" CACHE STRING "qcbor git refspec")
set(QCBOR_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/qcbor-src" CACHE PATH "qcbor installation directory")
set(QCBOR_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/qcbor_install" CACHE PATH "qcbor installation directory")

set(GIT_OPTIONS
	GIT_REPOSITORY ${QCBOR_URL}
	GIT_TAG ${QCBOR_REFSPEC}
	GIT_SHALLOW TRUE

	PATCH_COMMAND git stash
		COMMAND git branch -f bf-patch
		COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/0001-Add-3rd-party-settings.patch ${CMAKE_CURRENT_LIST_DIR}/0002-Add-install-definition.patch
		COMMAND git reset bf-patch
)


include(${TS_ROOT}/tools/cmake/common/LazyFetch.cmake REQUIRED)
LazyFetch_MakeAvailable(DEP_NAME qcbor
	FETCH_OPTIONS "${GIT_OPTIONS}"
	INSTALL_DIR "${QCBOR_INSTALL_DIR}"
	CACHE_FILE "${CMAKE_CURRENT_LIST_DIR}/qcbor-init-cache.cmake.in"
	SOURCE_DIR "${QCBOR_SOURCE_DIR}"
	)

# Create an imported target to have clean abstraction in the build-system.
add_library(qcbor STATIC IMPORTED)
set_property(TARGET qcbor PROPERTY IMPORTED_LOCATION "${QCBOR_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}qcbor${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(TARGET qcbor PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${QCBOR_INSTALL_DIR}/include")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${QCBOR_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}qcbor${CMAKE_STATIC_LIBRARY_SUFFIX}")

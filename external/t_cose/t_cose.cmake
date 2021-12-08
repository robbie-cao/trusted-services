#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# t_cose is a library for signing CBOR tokens using COSE_Sign1
#-------------------------------------------------------------------------------

set(T_COSE_URL "https://github.com/laurencelundblade/t_cose.git" CACHE STRING "t_cose repository URL")
set(T_COSE_REFSPEC "fc3a4b2c7196ff582e8242de8bd4a1bc4eec577f" CACHE STRING "t_cose git refspec")
set(T_COSE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/t_cose-src" CACHE PATH "t_cose installation directory")
set(T_COSE_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/t_cose_install" CACHE PATH "t_cose installation directory")

set(GIT_OPTIONS
	GIT_REPOSITORY ${T_COSE_URL}
	GIT_TAG ${T_COSE_REFSPEC}
	GIT_SHALLOW TRUE

	PATCH_COMMAND git stash
		COMMAND git branch -f bf-patch
		COMMAND git am ${CMAKE_CURRENT_LIST_DIR}/0001-add-install-definition.patch
		COMMAND git reset bf-patch
)

# Prepare include paths for dependencies that t_codse has on external components
get_target_property(_qcbor_inc qcbor INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND TCOSE_EXTERNAL_INCLUDE_PATHS ${_qcbor_inc} ${PSA_CRYPTO_API_INCLUDE})

include(${TS_ROOT}/tools/cmake/common/LazyFetch.cmake REQUIRED)
LazyFetch_MakeAvailable(DEP_NAME t_cose
	FETCH_OPTIONS "${GIT_OPTIONS}"
	INSTALL_DIR ${T_COSE_INSTALL_DIR}
	CACHE_FILE "${CMAKE_CURRENT_LIST_DIR}/t_cose-init-cache.cmake.in"
	SOURCE_DIR "${T_COSE_SOURCE_DIR}"
	)

unset(TCOSE_EXTERNAL_INCLUDE_PATHS)

# Create an imported target to have clean abstraction in the build-system.
add_library(t_cose STATIC IMPORTED)
target_link_libraries(t_cose INTERFACE qcbor)
set_property(TARGET t_cose PROPERTY IMPORTED_LOCATION "${T_COSE_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}t_cose${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(TARGET t_cose PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${T_COSE_INSTALL_DIR}/include")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${T_COSE_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}t_cose${CMAKE_STATIC_LIBRARY_SUFFIX}")

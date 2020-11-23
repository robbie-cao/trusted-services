#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
if (NOT DEFINED TGT)
	message(FATAL_ERROR "mandatory parameter TGT is not defined.")
endif()

target_sources(${TGT} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/crypto_client.cpp"
	)

# The crypto client presents the PSA Crypto API and hence has a dependency on mbedcrypto for functions
# related to setting key attributes.  A minimal configuration is provided to allow a minimal library
# to be built.  This configuration may be overridden by other components that have their own
# dependency on mbedctupto.
set(MBEDCRYPTO_CONFIG_FILE
	"${CMAKE_CURRENT_LIST_DIR}/config_mbed_crypto.h"
	CACHE STRING "Configuration file for mbedcrypto")


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
	"${CMAKE_CURRENT_LIST_DIR}/crypto_provider.c"
	)

target_include_directories(${TGT}
	 PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}"
	)

# Force use of the mbed crypto configuration required by the crypto service
# provider.  This configuration includes enabling the use of the PSA ITS API
# for persistent key storage which is realised by the its client adapter
# for the secure storage service.
set(MBEDCRYPTO_CONFIG_FILE
	"${CMAKE_CURRENT_LIST_DIR}/config_mbed_crypto.h"
	CACHE STRING "Configuration file for mbedcrypto" FORCE)

set(MBEDCRYPTO_EXTRA_INCLUDES
	"${TS_ROOT}/components/service/common"
	"${TS_ROOT}/components/service/secure_storage/client"
	CACHE STRING "PSA ITS for mbedcrypto" FORCE)

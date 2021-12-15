#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "smm-gateway"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/fdt"
		"components/common/trace"
		"components/common/utils"
		"components/common/uuid"
		"components/config/ramstore"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/ffarpc/endpoint"
		"components/rpc/ffarpc/caller/sp"
		"components/rpc/mm_communicate/endpoint/sp"
		"components/rpc/common/caller"
		"components/rpc/common/interface"
		"components/service/common/include"
		"components/service/common/client"
		"components/service/common/provider"
		"components/service/locator"
		"components/service/locator/interface"
		"components/service/locator/sp"
		"components/service/locator/sp/ffa"
		"components/service/smm_variable/backend"
		"components/service/smm_variable/frontend/mm_communicate"
		"components/service/smm_variable/provider"
		"components/service/secure_storage/include"
		"components/service/secure_storage/backend/secure_storage_client"
		"components/service/secure_storage/backend/mock_store"
		"protocols/rpc/common/packed-c"
		"protocols/service/secure_storage/packed-c"
)

target_sources(smm-gateway PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/smm_gateway_sp.c
	${CMAKE_CURRENT_LIST_DIR}/common/smm_gateway.c
)

target_include_directories(smm-gateway PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

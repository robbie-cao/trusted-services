#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "block-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/fdt"
		"components/common/tlv"
		"components/common/uuid"
		"components/common/endian"
		"components/common/trace"
		"components/common/utils"
		"components/config/ramstore"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/common/interface"
		"components/rpc/ffarpc/endpoint"
		"components/service/common/include"
		"components/service/common/provider"
		"components/service/block_storage/block_store"
		"components/service/block_storage/block_store/device"
		"components/service/block_storage/block_store/device/ram"
		"components/service/block_storage/block_store/partitioned"
		"components/service/block_storage/provider"
		"components/service/block_storage/provider/serializer/packed-c"
		"components/service/block_storage/config/ref"
		"components/service/block_storage/factory/ref_ram"
)

target_sources(block-storage PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/block_storage_sp.c
)

#################################################################

target_include_directories(block-storage PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "internal-trusted-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/common/trace
		components/common/utils
		components/messaging/ffa/libsp
		components/rpc/ffarpc/endpoint
		components/rpc/common/interface
		components/service/common/include
		components/service/common/provider
		components/service/secure_storage/include
		components/service/secure_storage/frontend/secure_storage_provider
		components/service/secure_storage/backend/secure_flash_store
		components/service/secure_storage/backend/secure_flash_store/flash_fs
		components/service/secure_storage/backend/secure_flash_store/flash
		components/service/secure_storage/backend/secure_flash_store/flash/ram
		components/service/secure_storage/factory/common/sfs
		protocols/rpc/common/packed-c
		protocols/service/secure_storage/packed-c
)

target_sources(internal-trusted-storage PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/its_sp.c
)

target_include_directories(internal-trusted-storage PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
	${TS_ROOT}/deployments/internal-trusted-storage/common
)

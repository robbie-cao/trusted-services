#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_components(TARGET "sfs-demo"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		components/common/trace
		components/common/utils
		components/messaging/ffa/libsp
		components/rpc/common/interface
		components/rpc/common/caller
		components/rpc/ts_rpc/common
		components/rpc/ts_rpc/caller/sp
		components/service/common/include
		components/service/common/client
		components/service/secure_storage/include
		components/service/secure_storage/frontend/psa/its
		components/service/secure_storage/backend/secure_storage_client
		protocols/rpc/common/packed-c
		protocols/service/secure_storage/packed-c
)

target_sources(sfs-demo PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/common/sfs_demo_sp.c
)

target_include_directories(sfs-demo PRIVATE
	${TS_ROOT}
	${TS_ROOT}/components
)

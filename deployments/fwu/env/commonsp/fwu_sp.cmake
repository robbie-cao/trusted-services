#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Includes components needed for deploying the fwu service provider
# within a secure partition.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Common components for fwu sp deployments
#
#-------------------------------------------------------------------------------
add_components(TARGET "fwu"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/common/fdt"
		"components/common/trace"
		"components/common/utils"
		"components/common/crc32/native"
		"components/config/ramstore"
		"components/config/loader/sp"
		"components/messaging/ffa/libsp"
		"components/rpc/common/interface"
		"components/rpc/ffarpc/endpoint"
		"components/service/common/provider"
		"components/service/fwu/provider"
		"components/service/fwu/provider/serializer/packed-c"
		"components/service/discovery/provider"
		"components/service/discovery/provider/serializer/packed-c"
)

target_sources(fwu PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/fwu_sp.c
)

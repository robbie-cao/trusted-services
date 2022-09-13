#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# A block-storage infrastructure for use on virtual platforms where storage
# is provided by file residing in the host's filesystem. If the file contains
# a GPT, it will be used to configure storage partitions.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Infrastructure components
#
#-------------------------------------------------------------------------------
add_components(TARGET "block-storage"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
		"components/service/block_storage/block_store/device"
		"components/service/block_storage/block_store/device/semihosting"
		"components/service/block_storage/block_store/partitioned"
		"components/service/block_storage/factory/semihosting"
)

#-------------------------------------------------------------------------------
#  This infrastructure depends on platform specific drivers
#
#-------------------------------------------------------------------------------
add_platform(TARGET "block-storage")
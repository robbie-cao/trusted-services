#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# A stub infrastructure for the se-proxy. Infrastructure dependencies are all
# realized with stub components that do absolutely nothing.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Infrastructure components
#
#-------------------------------------------------------------------------------
add_components(TARGET "se-proxy"
	BASE_DIR ${TS_ROOT}
	COMPONENTS
	"components/rpc/dummy"
	"components/rpc/common/caller"
	"components/rpc/rss_comms"
	"components/messaging/rss_comms/sp"
	"components/service/secure_storage/backend/secure_storage_ipc"
	"components/service/attestation/reporter/stub"
	"components/service/attestation/key_mngr/stub"
	"components/service/crypto/backend/psa_ipc"
	"components/service/crypto/client/psa"
	"components/service/secure_storage/backend/mock_store"
)

target_sources(se-proxy PRIVATE

	${CMAKE_CURRENT_LIST_DIR}/service_proxy_factory.c
)

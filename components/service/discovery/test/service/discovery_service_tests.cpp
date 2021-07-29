/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <service/discovery/client/discovery_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level tests for the discovery service.  The discovery service
 * provides common operations that may be called at all service endpoints.
 * These tests use the crypto service as the target.
 */
TEST_GROUP(DiscoveryServiceTests)
{
	void setup()
	{
		int status;

		m_rpc_session_handle = NULL;
		m_service_context = NULL;

		service_locator_init();

		m_service_context =
			service_locator_query("sn:trustedfirmware.org:crypto:0", &status);
		CHECK_TRUE(m_service_context);

		m_rpc_session_handle =
			service_context_open(m_service_context, TS_RPC_ENCODING_PACKED_C, &m_caller);
		CHECK_TRUE(m_rpc_session_handle);
	}

	void teardown()
	{
		service_context_close(m_service_context, m_rpc_session_handle);
		m_rpc_session_handle = NULL;

		service_context_relinquish(m_service_context);
		m_service_context = NULL;
	}

	struct rpc_caller *m_caller;
	rpc_session_handle m_rpc_session_handle;
	struct service_context *m_service_context;
};

TEST(DiscoveryServiceTests, checkServiceInfo)
{
	struct service_client service_client;
	psa_status_t status;

	status = service_client_init(&service_client, m_caller);
	LONGS_EQUAL(PSA_SUCCESS, status);

	status = discovery_client_get_service_info(&service_client);
	LONGS_EQUAL(PSA_SUCCESS, status);

	/* Check discovered service info looks reasonable */
	CHECK_TRUE(service_client.service_info.max_payload);
	CHECK_TRUE(service_client.service_info.supported_encodings);
}

/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <service/secure_storage/frontend/psa/ps/ps_frontend.h>
#include <service/secure_storage/frontend/psa/ps/test/ps_api_tests.h>
#include <service/secure_storage/frontend/psa/its/its_frontend.h>
#include <service/secure_storage/frontend/psa/its/test/its_api_tests.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level tests using the PSA Protected Storage and Internal
 * Trusted Storage APIs to access the secure storage service with the
 * 'protected-storage' designation.
 */
TEST_GROUP(PsServiceTests)
{
    void setup()
    {
        struct rpc_caller *caller;
        int status;

        m_rpc_session_handle = NULL;
        m_its_service_context = NULL;

        service_locator_init();

        m_its_service_context = service_locator_query("sn:trustedfirmware.org:protected-storage:0", &status);
        CHECK(m_its_service_context);

        m_rpc_session_handle = service_context_open(m_its_service_context, TS_RPC_ENCODING_PACKED_C, &caller);
        CHECK(m_rpc_session_handle);

        struct storage_backend *storage_backend = secure_storage_client_init(&m_storage_client, caller);

        psa_ps_frontend_init(storage_backend);
        psa_its_frontend_init(storage_backend);
    }

    void teardown()
    {
        psa_ps_frontend_init(NULL);
        psa_its_frontend_init(NULL);

	if (m_its_service_context) {
		if (m_rpc_session_handle) {
			service_context_close(m_its_service_context, m_rpc_session_handle);
			m_rpc_session_handle = NULL;
		}

		service_context_relinquish(m_its_service_context);
		m_its_service_context = NULL;
	}

        secure_storage_client_deinit(&m_storage_client);
    }

    rpc_session_handle m_rpc_session_handle;
    struct service_context *m_its_service_context;
    struct secure_storage_client m_storage_client;
};

TEST(PsServiceTests, storeNewItem)
{
    its_api_tests::storeNewItem();
}

TEST(PsServiceTests, createAndSet)
{
    ps_api_tests::createAndSet();
}

TEST(PsServiceTests, createAndSetExtended)
{
    ps_api_tests::createAndSetExtended();
}

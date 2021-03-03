/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <CppUTest/TestHarness.h>
#include <rpc/direct/direct_caller.h>
#include <service/secure_storage/frontend/psa/its/its_frontend.h>
#include <service/secure_storage/frontend/psa/its/test/its_api_tests.h>
#include <service/secure_storage/frontend/psa/ps/ps_frontend.h>
#include <service/secure_storage/frontend/psa/ps/test/ps_api_tests.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>


TEST_GROUP(SecureStorageClientTests)
{
    /* Runs ITS and PS API tests against a typical service configuration
     * comprising:
     * its/ps_frontend->secure_storage_client->secure_storage_provider->mock_store
     */
    void setup()
    {
        struct storage_backend *storage_provider_backend =
            mock_store_init(&m_mock_store);
        struct rpc_interface *storage_ep =
            secure_storage_provider_init(&m_storage_provider, storage_provider_backend);
        struct rpc_caller *storage_caller =
            direct_caller_init_default(&m_storage_caller, storage_ep);
        struct storage_backend *storage_client_backend =
            secure_storage_client_init(&m_storage_client, storage_caller);

        psa_its_frontend_init(storage_client_backend);
        psa_ps_frontend_init(storage_client_backend);
    }

    void teardown()
    {
        mock_store_deinit(&m_mock_store);
        secure_storage_provider_deinit(&m_storage_provider);
        secure_storage_client_deinit(&m_storage_client);
        direct_caller_deinit(&m_storage_caller);
    }

    struct mock_store m_mock_store;
    struct secure_storage_provider m_storage_provider;
    struct secure_storage_client m_storage_client;
    struct direct_caller m_storage_caller;
};

TEST(SecureStorageClientTests, itsStoreNewItem)
{
    its_api_tests::storeNewItem();
}

TEST(SecureStorageClientTests, itsStorageLimitTest)
{
    its_api_tests::storageLimitTest(MOCK_STORE_ITEM_SIZE_LIMIT);
}

TEST(SecureStorageClientTests, psCreateAndSet)
{
    ps_api_tests::createAndSet();
}

TEST(SecureStorageClientTests, psCreateAndSetExtended)
{
    ps_api_tests::createAndSetExtended();
}

/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstring>
#include <cstdint>
#include <CppUTest/TestHarness.h>
#include <rpc/direct/direct_caller.h>
#include <service/secure_storage/frontend/psa/its/its_frontend.h>
#include <service/secure_storage/backend/secure_storage_client/secure_storage_client.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <service/secure_storage/backend/secure_flash_store/secure_flash_store.h>
#include <psa/internal_trusted_storage.h>
#include <psa/error.h>

TEST_GROUP(InternalTrustedStorageTests)
{
    void setup()
    {
        struct storage_backend *storage_provider_backend = sfs_init();
        struct rpc_interface *storage_ep = secure_storage_provider_init(&m_storage_provider, storage_provider_backend);
        struct rpc_caller *storage_caller = direct_caller_init_default(&m_storage_caller, storage_ep);

        struct storage_backend *storage_client_backend = secure_storage_client_init(&m_storage_client, storage_caller);
        psa_its_frontend_init(storage_client_backend);
    }

    void teardown()
    {
        secure_storage_provider_deinit(&m_storage_provider);
        secure_storage_client_deinit(&m_storage_client);
        direct_caller_deinit(&m_storage_caller);
    }

    struct secure_storage_provider m_storage_provider;
    struct secure_storage_client m_storage_client;
    struct direct_caller m_storage_caller;
};

TEST(InternalTrustedStorageTests, storeNewItem)
{
    psa_status_t status;
    psa_storage_uid_t uid = 10;
    struct psa_storage_info_t storage_info;
    static const size_t ITEM_SIZE = 68;
    uint8_t item[ITEM_SIZE];
    uint8_t read_item[ITEM_SIZE];

    memset(item, 0x55, sizeof(item));

    /* Probe to check item does not exist */
    status = psa_its_get_info(uid, &storage_info);
    CHECK_EQUAL(PSA_ERROR_DOES_NOT_EXIST, status);

    /* Store the item */
    status = psa_its_set(uid, sizeof(item), item, PSA_STORAGE_FLAG_NONE);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Probe to check item now exists */
    status = psa_its_get_info(uid, &storage_info);
    CHECK_EQUAL(PSA_SUCCESS, status);
    CHECK_EQUAL(sizeof(item), storage_info.size);

    /* Get the item */
    size_t read_len = 0;
    status = psa_its_get(uid, 0, sizeof(read_item), read_item, &read_len);
    CHECK_EQUAL(PSA_SUCCESS, status);
    CHECK_EQUAL(sizeof(item), read_len);
    CHECK(memcmp(item, read_item, sizeof(item)) == 0);

    /* Remove the item */
    status = psa_its_remove(uid);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Expect it to have gone */
    status = psa_its_get_info(uid, &storage_info);
    CHECK_EQUAL(PSA_ERROR_DOES_NOT_EXIST, status);
}

TEST(InternalTrustedStorageTests, storageLimitTest)
{
    psa_status_t status;
    psa_storage_uid_t uid = 10;
    struct psa_storage_info_t storage_info;
    static const size_t MAX_ITEM_SIZE = 10000;
    uint8_t item[MAX_ITEM_SIZE];
    uint8_t read_item[MAX_ITEM_SIZE];

    memset(item, 0x55, sizeof(item));

    /* Probe to check item does not exist */
    status = psa_its_get_info(uid, &storage_info);
    CHECK_EQUAL(PSA_ERROR_DOES_NOT_EXIST, status);

    /* Attempt to store a reasonably big item */
    status = psa_its_set(uid, 5000, item, PSA_STORAGE_FLAG_NONE);
    CHECK(PSA_SUCCESS != status);

    /* Attempt to store a stupidly big item */
    status = psa_its_set(uid, static_cast<size_t>(-1), item, PSA_STORAGE_FLAG_NONE);
    CHECK(PSA_SUCCESS != status);

    /* Attempt to store a zero length item */
    status = psa_its_set(uid, 0, item, PSA_STORAGE_FLAG_NONE);
    CHECK_EQUAL(PSA_SUCCESS, status);

    /* Remove the item */
    status = psa_its_remove(uid);
    CHECK_EQUAL(PSA_SUCCESS, status);
}
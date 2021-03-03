/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <CppUTest/TestHarness.h>
#include <service/secure_storage/frontend/psa/its/its_frontend.h>
#include <service/secure_storage/frontend/psa/its/test/its_api_tests.h>
#include <service/secure_storage/frontend/psa/ps/ps_frontend.h>
#include <service/secure_storage/frontend/psa/ps/test/ps_api_tests.h>
#include <service/secure_storage/backend/secure_flash_store/secure_flash_store.h>


TEST_GROUP(SfsTests)
{
    void setup()
    {
        struct storage_backend *storage_backend = sfs_init();

        psa_its_frontend_init(storage_backend);
        psa_ps_frontend_init(storage_backend);
    }
};

TEST(SfsTests, itsStoreNewItem)
{
    its_api_tests::storeNewItem();
}

TEST(SfsTests, itsStorageLimitTest)
{
    its_api_tests::storageLimitTest(5000);
}

TEST(SfsTests, psCreateAndSet)
{
    ps_api_tests::createAndSet();
}

TEST(SfsTests, psCreateAndSetExtended)
{
    ps_api_tests::createAndSetExtended();
}

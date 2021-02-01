/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstring>
#include <config/ramstore/config_ramstore.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(ConfigRamstoreTests)
{
    void setup()
    {
        config_ramstore_init();
    }

    void teardown()
    {
        config_ramstore_deinit();
    }
};

TEST(ConfigRamstoreTests, checkEmptyConfig)
{
    /* Expect queries to an empty store to return gracefully */
    struct device_region *query_result = platform_config_device_query("flash", 0);
    CHECK(!query_result);

    /* Expect freeing a null pointer to be harmless */
    platform_config_device_query_free(query_result);
}

TEST(ConfigRamstoreTests, checkSingleConfig)
{
    struct device_region config;

    /* This would be external configuration, obtained say from device tree */
    strcpy(config.dev_class, "fs");
    config.dev_instance = 2;
    config.base_addr = (uint8_t*)0x0f000010;
    config.io_region_size = 0x100;

    /* Add the configuration object */
    int status = platform_config_device_add(&config);
    CHECK_EQUAL(0, status);

    /* Expect query find the config object */
    struct device_region *query_result = platform_config_device_query(config.dev_class, config.dev_instance);
    CHECK(query_result);
    CHECK(strcmp(config.dev_class, query_result->dev_class) == 0);
    CHECK_EQUAL(config.dev_instance, query_result->dev_instance);
    CHECK_EQUAL(config.base_addr, query_result->base_addr);
    CHECK_EQUAL(config.io_region_size, query_result->io_region_size);

    platform_config_device_query_free(query_result);
}

TEST(ConfigRamstoreTests, checkMultipleConfig)
{
    int status;

    /* Add first config object */
    struct device_region config1;

    strcpy(config1.dev_class, "flash");
    config1.dev_instance = 0;
    config1.base_addr = (uint8_t*)0x0f000010;
    config1.io_region_size = 0x100;

    status = platform_config_device_add(&config1);
    CHECK_EQUAL(0, status);

    /* Add second config object */
    struct device_region config2;

    strcpy(config2.dev_class, "flash");
    config2.dev_instance = 1;
    config2.base_addr = (uint8_t*)0x0f000010;
    config2.io_region_size = 0x100;

    status = platform_config_device_add(&config2);
    CHECK_EQUAL(0, status);

    /* Expect queries for both objects to work */
    struct device_region *query1_result = platform_config_device_query(config1.dev_class, config1.dev_instance);
    CHECK(query1_result);

    struct device_region *query2_result = platform_config_device_query(config2.dev_class, config2.dev_instance);
    CHECK(query2_result);

    platform_config_device_query_free(query2_result);
}
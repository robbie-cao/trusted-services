/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <service/test_runner/provider/backend/simple_c/simple_c_test_runner.h>
#include <config/interface/platform_config.h>
#include <stdint.h>

/**
 * Secure Partition configuration tests for checking configuartion
 * data passed to an SP at initialisation.  These tests assume
 * use of the FFA manifest for any SP deployments of
 * deployments/env_test.
 */

/*
 * Check that the loaded configuration includes one or more
 * device regions.
 */
static bool check_device_region_loaded(struct test_failure *failure)
{
    return platform_config_device_region_count() > 0;
}

/*
 * Check that a device region for a 'trng' device has been loaded
 * and that values are as expected.
 */
static bool check_trng_device_region_loaded(struct test_failure *failure)
{
    bool passed = false;
    struct device_region *dev_region = platform_config_device_query("trng", 0);

    if (dev_region) {

        passed =
            (dev_region->dev_instance == 0) &&
            (dev_region->io_region_size == 0x1000);
    }

    platform_config_device_query_free(dev_region);

    return passed;
}

/*
 * Check access to some trng registers
 */
static bool check_trng_register_access(struct test_failure *failure)
{
    bool passed = false;

    struct device_region *dev_region = platform_config_device_query("trng", 0);

    if (dev_region) {

        /* Expect reset values to be read from a selection of TRNG registers */
        uint32_t reg_val;
        passed = true;

        /* PID4 */
        if (passed) {
            reg_val = *((volatile uint32_t*)((uint8_t*)dev_region->base_addr + 0xfd0));
            passed = (reg_val == 0x00000004);
            failure->line_num = __LINE__;
            failure->info = reg_val;
        }

        /* PID0 */
        if (passed) {
            reg_val = *((volatile uint32_t*)((uint8_t*)dev_region->base_addr + 0xfe0));
            passed = (reg_val == 0x000000aa);
            failure->line_num = __LINE__;
            failure->info = reg_val;
        }
    }

    return passed;
}


/**
 * Define an register test group
 */
void sp_config_tests_register(void)
{
    static const struct simple_c_test_case sp_config_tests[] = {
        {.name = "DevRegionLoaded", .test_func = check_device_region_loaded},
        {.name = "TrngDevRegionLoaded", .test_func = check_trng_device_region_loaded},
        {.name = "TrngRegAccess", .test_func = check_trng_register_access}
    };

    static const struct simple_c_test_group sp_config_test_group =
    {
        .group = "SpConfigTests",
        .num_test_cases = sizeof(sp_config_tests)/sizeof(struct simple_c_test_case),
        .test_cases = sp_config_tests
    };

    simple_c_test_runner_register_group(&sp_config_test_group);
}
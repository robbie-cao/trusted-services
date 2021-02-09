// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <service/test_runner/provider/backend/simple_c/simple_c_test_runner.h>
#include <config/test/sp/sp_config_tests.h>
#include <service/crypto/provider/mbedcrypto/trng_adapter/test/trng_env_tests.h>

void env_test_register_tests(struct test_runner_provider *context)
{
	simple_c_test_runner_init(context);

	sp_config_tests_register();
	trng_env_tests_register();
}
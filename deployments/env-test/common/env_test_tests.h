/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ENV_TEST_TESTS_H
#define ENV_TEST_TESTS_H

struct test_runner_provider;

/**
 * Registers test cases for this deployment
 */
void env_test_register_tests(struct test_runner_provider *context);


#endif /* ENV_TEST_TESTS_H */

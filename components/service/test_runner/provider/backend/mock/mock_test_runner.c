/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <service/test_runner/provider/test_runner_backend.h>
#include <service/test_runner/provider/test_runner_provider.h>
#include <stdbool.h>
#include <string.h>

/**
 * The mock backend is a test_runner that provides some mock test cases
 * that can be used for testing the test_runner service iteslf.
 */

struct mock_test_case
{
    const char *group;
    const char *name;
    bool (*test_func)(void);
};

/* Mock test test functions */
static bool test_that_passes(void)  { return true; }
static bool test_that_fails(void)   { return false; }

/* Mock test suite */
const struct mock_test_case mock_test_suite[] =
{
    {.group = "PlatformTests", .name = "Trng", .test_func = test_that_passes},
    {.group = "PlatformTests", .name = "CheckIOmap", .test_func = test_that_passes},
    {.group = "ConfigTests", .name = "ValidateConfig", .test_func = test_that_fails},
    {.group = "ConfigTests", .name = "ApplyConfig", .test_func = test_that_passes}
};


static bool does_qualify(const struct mock_test_case *test_case, const struct test_spec *spec)
{
    return
        ((strlen(spec->group) == 0) || (strcmp(spec->group, test_case->group) == 0)) &&
        ((strlen(spec->name) == 0) || (strcmp(spec->name, test_case->name) == 0));
}

static size_t count_tests(const struct test_spec *spec)
{
    size_t count = 0;

    for (size_t i = 0; i < sizeof(mock_test_suite)/sizeof(struct mock_test_case); ++i) {

        if (does_qualify(&mock_test_suite[i], spec)) ++count;
    }

    return count;
}

static int run_tests(const struct test_spec *spec, struct test_summary *summary,
                    struct test_result *results, size_t result_limit)
{
    summary->num_tests = 0;
	summary->num_results = 0;
	summary->num_passed = 0;
	summary->num_failed = 0;

    for (size_t i = 0; i < sizeof(mock_test_suite)/sizeof(struct mock_test_case); ++i) {

        if (does_qualify(&mock_test_suite[i], spec)) {

            bool did_pass = mock_test_suite[i].test_func();

            if (did_pass)
                ++summary->num_passed;
            else
                ++summary->num_failed;

            if (summary->num_tests < result_limit) {

                struct test_result *new_result = &results[summary->num_results];

                new_result->run_state = (did_pass) ? TEST_RUN_STATE_PASSED : TEST_RUN_STATE_FAILED;
                new_result->fail_line = 0;
                strcpy(new_result->group, mock_test_suite[i].group);
                strcpy(new_result->name, mock_test_suite[i].name);

                ++summary->num_results;
            }

            ++summary->num_tests;
        }
    }

    return 0;
}

static void list_tests(const struct test_spec *spec, struct test_summary *summary,
                    struct test_result *results, size_t result_limit)
{
    summary->num_tests = 0;
	summary->num_results = 0;
	summary->num_passed = 0;
	summary->num_failed = 0;

    for (size_t i = 0; i < sizeof(mock_test_suite)/sizeof(struct mock_test_case); ++i) {

        if (does_qualify(&mock_test_suite[i], spec)) {

            if (summary->num_tests < result_limit) {

                struct test_result *new_result = &results[summary->num_results];

                new_result->run_state = TEST_RUN_STATE_NOT_RUN;
                new_result->fail_line = 0;
                strcpy(new_result->group, mock_test_suite[i].group);
                strcpy(new_result->name, mock_test_suite[i].name);

                ++summary->num_results;
            }

            ++summary->num_tests;
        }
    }
}

void test_runner_register_default_backend(struct test_runner_provider *context)
{
    static struct test_runner_backend this_instance;

    this_instance.count_tests = count_tests;
    this_instance.run_tests = run_tests;
    this_instance.list_tests = list_tests;
    this_instance.next = NULL;

    test_runner_provider_register_backend(context, &this_instance);
}
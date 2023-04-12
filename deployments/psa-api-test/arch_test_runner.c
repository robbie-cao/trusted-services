/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rpc/common/logging/logging_caller.h"
#include "service_under_test.h"
#include "service_locator.h"

#define TEST_ID_OFFSET      (5)
#define TEST_POSTFIX_OFFSET (8)
#define TEST_ENTRY_LENGTH   (9)


int32_t val_entry(void);
extern size_t val_get_test_list(uint32_t *test_id_list, size_t size);
extern void pal_set_custom_test_list(char *custom_test_list);

/* Returns whether option_switch is in the argv list and provide its index in the array */
static bool option_selected(const char *option_switch, int argc, char *argv[], int *index)
{
    bool selected = false;
    *index = 0;

    for (int i = 1; (i < argc) && !selected; ++i) {

        selected = (strcmp(argv[i], option_switch) == 0);
        *index = i;
    }

    return selected;
}

/* Print the supported command line arguments */
static void print_help(void)
{
    printf("Supported command line arguments:\n\n");
    printf("\t -l: Print list of tests.\n");
    printf("\t -t <test_list>: Run only the listed tests (e.g: test_201;test_202;). test_list = ^(test_[0-9]{3};)+ \n");
    printf("\t -v: Verbose mode.\n");
    printf("\t -h: Print this help message.\n");
    printf("\n");
}

/* Prints the list of selectable psa-api tests */
static void print_psa_api_tests(void)
{
    /* Request the number of tests to find out the size of the area needed to store the test ID-s. */
    size_t n_test = val_get_test_list(NULL, 0);

    uint32_t *test_id_list = (uint32_t *)calloc(n_test, sizeof(uint32_t));

    if (test_id_list) {
        n_test = val_get_test_list(test_id_list, n_test);

        printf("Available psa-api tests:\n");
        for (int i = 0; i < n_test; i++) {
                printf("\t test_%d;\n", test_id_list[i]);
        }

        free(test_id_list);
    }
    else {
        printf("Could not allocate enough memory to store the list of tests\n");
    }
}

/* Check if the received test list string is formatted as expected */
static bool is_test_list_wrong(char* test_list)
{
    size_t len = strlen(test_list);

    for (unsigned i = 0; i < len; i += TEST_ENTRY_LENGTH) {

        /* Report error when the test entry is not properly finished */
        if (i + TEST_ENTRY_LENGTH > len) {
            printf("Expecting \"test_xxx;\" test entry at the %dth character, got \"%s\" instead.\n", i, &test_list[i]);
            return true;
        }

        /* Report error at incorrect test entry prefix */
        if (memcmp(&test_list[i], "test_", TEST_ID_OFFSET)) {
            printf("Expecting \"test_\" at the %dth character, got \"%.5s\" instead.\n", i, &test_list[i]);
            return true;
        }

        /* Report error if the test ID is incorrect */
        if (!(isdigit(test_list[i + TEST_ID_OFFSET]) &&
              isdigit(test_list[i + TEST_ID_OFFSET + 1]) &&
              isdigit(test_list[i + TEST_ID_OFFSET + 2]))) {
            printf("Expecting three digits at the %dth character, got \"%.3s\" instead.\n",
                i + TEST_ID_OFFSET,
                &test_list[i + TEST_ID_OFFSET]);
            return true;
        }

        /* Report error at incorrect test entry postfix */
        if (test_list[i + TEST_POSTFIX_OFFSET] != ';') {
            printf("Expecting ; at the %dth character, got \"%.1s\" instead.\n",
                i + TEST_POSTFIX_OFFSET,
                &test_list[i + TEST_POSTFIX_OFFSET]);
            return true;
        }
    }

    return false;
}

/* Entry point */
int main(int argc, char *argv[])
{
    int rval = -1;
    int option_index = 0;
    struct logging_caller *selected_call_logger = NULL;
    struct logging_caller call_logger;

    logging_caller_init(&call_logger, stdout);
    service_locator_init();

     /* Print available tests */
    if (option_selected("-l", argc, argv, &option_index)) {
        print_psa_api_tests();
        return 0;
    }

    /* Create custom test list */
    if (option_selected("-t", argc, argv, &option_index)) {
        /* Avoid overindexing of argv and detect if the option is followed by another option */
        char *test_list_values = argv[option_index + 1];
        if ((option_index >= argc) || (test_list_values[0] == '-')) {
            printf("Testlist string is expected after -t argument!\n");
            return -1;
        }

        if (is_test_list_wrong(test_list_values)) {
            printf("Testlist string is not valid!\n");
            print_psa_api_tests();
            return -1;
        }

        /* Filter tests */
        pal_set_custom_test_list(test_list_values);
    }

    /* Setup verbose mode */
    if (option_selected("-v", argc, argv, &option_index))
        selected_call_logger = &call_logger;

    /* Print help */
    if (option_selected("-h", argc, argv, &option_index)) {
        print_help();
        return 0;
    }

    /* Locate service under test */
    rval = locate_service_under_test(selected_call_logger);

    /* Run tests */
    if (!rval) {

        rval = val_entry();

        relinquish_service_under_test();
    }
    else {

        printf("Failed to locate service under test.  Error code: %d\n", rval);
    }

    logging_caller_deinit(&call_logger);

    return rval;
}

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <service_locator.h>
#include <rpc/common/logging/logging_caller.h>
#include "service_under_test.h"

int32_t val_entry(void);

static bool option_selected(const char *option_switch, int argc, char *argv[])
{
    bool selected = false;

    for (int i = 1; (i < argc) && !selected; ++i) {

        selected = (strcmp(argv[i], option_switch) == 0);
    }

    return selected;
}

int main(int argc, char *argv[])
{
    int rval = -1;
    struct logging_caller *selected_call_logger = NULL;
    struct logging_caller call_logger;

    logging_caller_init(&call_logger, stdout);
    service_locator_init();

    /* Check command line options */
    if (option_selected("-l", argc, argv)) selected_call_logger = &call_logger;

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

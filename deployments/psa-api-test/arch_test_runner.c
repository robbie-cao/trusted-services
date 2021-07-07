/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <service_locator.h>
#include "service_under_test.h"

int32_t val_entry(void);

int main(int argc, char *argv[])
{
    int rval = -1;

    service_locator_init();

    rval = locate_service_under_test();

    if (!rval) {

        rval = val_entry();

        relinquish_service_under_test();
    }
    else {

        printf("Failed to locate service under test.  Error code: %d\n", rval);
    }

    return rval;
}

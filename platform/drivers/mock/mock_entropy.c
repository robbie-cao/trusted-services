/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <platform/interface/entropy.h>

/*
 * A platform entropy driver that provides a mock implementation that
 * always returns a fixed value.  Intended for test purposes only.
 */
static int mock_poll(void *context, unsigned char *output, size_t nbyte, size_t *len)
{
    (void)context;
    (void)output;

    *len = 0;

    if (nbyte < sizeof(unsigned char) )
        return 0;

    *len = sizeof(unsigned char);

    return 0;
}

int ts_plat_entropy_create(struct ts_plat_entropy_driver *driver, void *config)
{
    static const struct ts_plat_entropy_iface iface =  { .poll = mock_poll };

    (void)config;

    driver->context = NULL;
    driver->iface = &iface;

    return 0;
}

void ts_plat_entropy_destroy(struct ts_plat_entropy_driver *driver)
{
    (void)driver;
}

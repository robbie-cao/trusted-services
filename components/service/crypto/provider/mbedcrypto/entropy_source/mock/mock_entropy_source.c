/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mbedtls/entropy_poll.h>
#include <stdint.h>

/*
 * A mock entropy source without any hardware dependencies.  Should not be
 * used in production deployments.
 */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    ((void) data);
    ((void) output);
    *olen = 0;

    if (len < sizeof(unsigned char) )
        return (0);

    *olen = sizeof(unsigned char);

    return (0);
}

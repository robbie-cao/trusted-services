/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <psa/crypto.h>
#include <service_locator.h>

int32_t val_entry(void);

int main(int argc, char *argv[])
{
    int rval = -1;

    psa_crypto_init();
    service_locator_init();

    rval = val_entry();

    return rval;
}

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mbedtls/entropy.h>
#include <mbedtls/entropy_poll.h>
#include <service/crypto/backend/mbedcrypto/trng_adapter/trng_adapter.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

/*
 * An mbed tls compatibile hardware entropy source that adapts the mbed tls hardware poll
 * function to the Linux getrandom system call.
 */

int trng_adapter_init(int instance)
{
    (void)instance;
}

void trng_adapter_deinit()
{

}

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    int status = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    *olen = 0;

    int num_output = syscall(SYS_getrandom, output, len, 0);

    if (num_output >= 0) {

        *olen = num_output;
        status = 0;
    }

    return status;
}

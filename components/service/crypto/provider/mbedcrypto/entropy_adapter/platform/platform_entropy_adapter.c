/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mbedtls/entropy.h>
#include <mbedtls/entropy_poll.h>
#include <platform/interface/entropy.h>
#include <service/crypto/provider/mbedcrypto/entropy_adapter/entropy_adapter.h>
#include <stddef.h>

/*
 * An mbed tls compatibile hardware entropy source that adapts the mbed tls hardware poll
 * function to a platform entropy driver.  The actual realization of the driver
 * will depend on the platform selected at build-time.
 */
static struct ts_plat_entropy_driver driver = {0};

int entropy_adapter_init(void *config)
{
    return ts_plat_entropy_create(&driver, config);
}

void entropy_adapter_deinit(void)
{
    ts_plat_entropy_destroy(&driver);

    driver.iface = NULL;
    driver.context = NULL;
}

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    int status = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    *olen = 0;

    if (driver.iface) {

        status = driver.iface->poll(driver.context, output, len, olen);
    }

    return status;
}

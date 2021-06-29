/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mbedtls/entropy.h>
#include <platform/interface/trng.h>
#include <service/crypto/backend/mbedcrypto/trng_adapter/trng_adapter.h>
#include <config/interface/config_store.h>
#include <psa/error.h>
#include <stddef.h>

/*
 * An mbed tls compatibile hardware entropy source that adapts the mbed tls hardware poll
 * function to a platform trng driver.  The actual realization of the driver
 * will depend on the platform selected at build-time.
 */
static struct platform_trng_driver driver = {0};

int trng_adapter_init(int instance)
{
	int status = PSA_STATUS_HARDWARE_FAILURE;
	struct device_region device_region;

	if (config_store_query(CONFIG_CLASSIFIER_DEVICE_REGION,
		"trng", instance,
		&device_region, sizeof(device_region))) {

		status = platform_trng_create(&driver, &device_region);
	}

	return status;
}

void trng_adapter_deinit()
{
	platform_trng_destroy(&driver);

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

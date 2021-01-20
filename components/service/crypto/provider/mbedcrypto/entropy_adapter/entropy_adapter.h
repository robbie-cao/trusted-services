/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MBED_CRYPTO_ENTROPY_ADAPTER_H
#define MBED_CRYPTO_ENTROPY_ADAPTER_H

/*
 * The build-time configuration of Mbed Crypto creates a dependency on a
 * hardware-based entropy source that provides an implementation of the
 * mbedtls_hardware_poll function.  Depending on the environment, this
 * could be realized in different ways e.g. via a native environment
 * specific service or using a platform specific driver.  This header
 * file defines the common interface for initializing and configuring
 * the adapter that provides the entropy source.
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialise the entropy adapter
 *
 * \param config    Entropy adapter specific configuration or NULL if none.
 *
 * \return          0 if successful.
 */
int entropy_adapter_init(void *config);

/**
 * \brief Cleans-up the entropy adapter.
 */
void entropy_adapter_deinit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MBED_CRYPTO_ENTROPY_ADAPTER_H */

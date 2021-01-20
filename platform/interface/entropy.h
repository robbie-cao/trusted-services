/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_PLATFORM_INTERFACE_ENTROPY_H
#define TS_PLATFORM_INTERFACE_ENTROPY_H

/*
 * Interface definintion for a platform entropy driver.  A platform provider will
 * provide concrete implementations of this interface for each alternative
 * implementation supported.
 */
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Virtual interface for a platform entropy driver.  A platform will provide
 * one or more concrete implementations of this interface.
 */
struct ts_plat_entropy_iface
{
   /**
    * \brief Poll for bytes of entropy from a platform entropy source
    *
    * \param context     Platform driver context
    * \param output      Buffer for output
    * \param nbyte       Desired number of bytes
    * \param len         The number of bytes returned (could be zero)
    *
    * \return            0 if successful.
    */
    int (*poll)(void *context, unsigned char *output, size_t nbyte, size_t *len);
};

/*
 * A platform entropy driver instance.
 */
struct ts_plat_entropy_driver
{
    void *context;                              /**< Opaque driver context */
    const struct ts_plat_entropy_iface *iface;  /**< Interface methods */
};

/**
 * \brief Factory method to construct a platform specific entropy driver
 *
 * \param driver    Pointer to driver structure to initialize on construction.
 * \param config    Driver specific configuration or NULL if none.
 *
 * \return          0 if successful.
 */
int ts_plat_entropy_create(struct ts_plat_entropy_driver *driver, void *config);

/**
 * \brief Destroy a driver constructed using the factory method
 *
 * \param driver    Pointer to driver structure for constructed driver.
 */
void ts_plat_entropy_destroy(struct ts_plat_entropy_driver *driver);

#ifdef __cplusplus
}
#endif

#endif /* TS_PLATFORM_INTERFACE_ENTROPY_H */

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_CONFIG_INTERFACE_PLATFORM_CONFIG_H
#define TS_CONFIG_INTERFACE_PLATFORM_CONFIG_H

#include <platform/interface/device_region.h>
#include <stddef.h>

/**
 * Provides a common interface for retrieving platform configuration
 * data for initializing platform provided devices or services.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Query platform configuartion for a particular device_region
 *
 * \param[in]  dev_class     Class of device (e.g. 'trng')
 * \param[in]  dev_instance  The instance of the class of a device on platform
 *
 * \return          Pointer to device_region or NULL if no qualifying configuration
 */
struct device_region *platform_config_device_query(const char *dev_class,
                                                    int dev_instance);

/**
 * \brief Frees a device region returned by platform_config_device_query()
 *
 * \param[in]  device_region   Device region object to free.  Can be NULL.
 */
void platform_config_device_query_free(struct device_region *device_region);

/**
 * \brief Add a device_region to the platform configuration
 *
 * \param[in] device_region The device_region object to add
 *
  * \return          0 if successful
 */
int platform_config_device_add(const struct device_region *device_region);


#ifdef __cplusplus
}
#endif

#endif /* TS_CONFIG_INTERFACE_PLATFORM_CONFIG_H */

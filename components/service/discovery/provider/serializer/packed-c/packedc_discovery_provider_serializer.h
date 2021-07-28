/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PACKEDC_DISCOVERY_PROVIDER_SERIALIZER_H
#define PACKEDC_DISCOVERY_PROVIDER_SERIALIZER_H

#include <service/discovery/provider/serializer/discovery_provider_serializer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Singleton method to provide access to the packed-c serializer
 * for the discovery service provider.
 */
const struct discovery_provider_serializer *packedc_discovery_provider_serializer_instance(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PACKEDC_DISCOVERY_PROVIDER_SERIALIZER_H */

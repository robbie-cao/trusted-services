/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PACKEDC_FWU_PROVIDER_SERIALIZER_H
#define PACKEDC_FWU_PROVIDER_SERIALIZER_H

#include "service/fwu/provider/serializer/fwu_provider_serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Singleton method to provide access to the packed-c serializer
 * for the fwu service provider.
 */
const struct fwu_provider_serializer *packedc_fwu_provider_serializer_instance(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PACKEDC_FWU_PROVIDER_SERIALIZER_H */

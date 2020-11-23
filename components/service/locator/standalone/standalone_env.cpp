/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <service_locator.h>
#include <service/locator/standalone/services/crypto/crypto_service_context.h>
#include "standalone_location_strategy.h"
#include "standalone_service_registry.h"

void service_locator_envinit(void)
{
    static crypto_service_context crypto_context("sn:trustedfirmware.org:crypto:0");
    standalone_service_registry::instance()->regsiter_service_instance(&crypto_context);
    service_locator_register_strategy(standalone_location_strategy());
}
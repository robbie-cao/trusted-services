/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_DEMO_H
#define TS_DEMO_H

#include <service/crypto/client/cpp/crypto_client.h>

int run_ts_demo(crypto_client *crypto_client, bool is_verbose);

#endif /* TS_DEMO_H */

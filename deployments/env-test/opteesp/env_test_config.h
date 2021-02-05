/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ENV_TEST_CONFIG_H
#define ENV_TEST_CONFIG_H

#include <ffa_api.h>

/**
 * Loads the SP specific configuration passed as SP initialization parameters.
 */
void load_sp_config(struct ffa_init_info *init_info);


#endif /* ENV_TEST_CONFIG_H */

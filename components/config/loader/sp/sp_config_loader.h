/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SP_CONFIG_LOADER_H
#define SP_CONFIG_LOADER_H

#include <ffa_api.h>

/**
 * Loads the secure partition specific configuration passed as
 * SP initialization parameters.
 */
void sp_config_load(struct ffa_init_info *init_info);


#endif /* SP_CONFIG_LOADER_H */

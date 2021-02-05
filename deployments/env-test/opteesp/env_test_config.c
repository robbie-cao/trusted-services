// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 */

#include <config/ramstore/config_ramstore.h>
#include "env_test_config.h"


void load_sp_config(struct ffa_init_info *init_info)
{
	config_ramstore_init();

	/* Load deployment specific configuration */
	(void)init_info;
}
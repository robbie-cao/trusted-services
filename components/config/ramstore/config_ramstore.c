/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_ramstore.h"
#include <config/interface/platform_config.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * Variable length container for a configuration object.
 */
struct config_container
{
	size_t size;
	struct config_container *next;
};

static struct config_container *config_container_create(const void *data, size_t size)
{
	struct config_container *container = malloc(sizeof(struct config_container) + size);

	if (container) {

		container->size = size;
		container->next = NULL;

		memcpy((uint8_t*)container + sizeof(struct config_container), data, size);
	}

	return container;
}

static void config_container_destroy(struct config_container *container)
{
	free(container);
}

static const void *config_container_data(const struct config_container *container)
{
	return (const uint8_t*)container + sizeof(struct config_container);
}

/**
 * Singleton config_ramstore instance
 */
static struct config_ramstore
{
	struct config_container *device_region_list;
} ramstore = {0};


void config_ramstore_init(void)
{
	ramstore.device_region_list = NULL;
}

void config_ramstore_deinit(void)
{
	while (ramstore.device_region_list) {

		struct config_container *next = ramstore.device_region_list->next;
		free(ramstore.device_region_list);
		ramstore.device_region_list = next;
	}
}

int platform_config_device_add(const struct device_region *device_region)
{
	struct config_container *container;

	container = config_container_create(device_region, sizeof(struct device_region));
	if (!container) return -1;

	container->next = ramstore.device_region_list;
	ramstore.device_region_list = container;

	return 0;
}

struct device_region *platform_config_device_query(const char *dev_class,
                                                    int dev_instance)
{
	struct device_region *result = NULL;
	const struct config_container *container = ramstore.device_region_list;

	while (container) {

		const struct device_region *candidate;
		candidate = (const struct device_region*)config_container_data(container);

		if ((candidate->dev_instance == dev_instance) &&
			(strcmp(candidate->dev_class, dev_class) == 0)) {

			result = malloc(container->size);
			if (result) {

				memcpy(result, candidate, container->size);
			}

			break;
		}

		container = container->next;
	}

	return result;
}

void platform_config_device_query_free(struct device_region *device_region)
{
	free(device_region);
}

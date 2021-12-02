/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include "variable_index_iterator.h"

void variable_index_iterator_first(
	struct variable_index_iterator *iter,
	const struct variable_index *variable_index)
{
	iter->variable_index = variable_index;
	iter->current_pos = variable_index->max_variables;

	for (size_t pos = 0; pos < variable_index->max_variables; pos++) {

		if (variable_index->entries[pos].in_use) {

			iter->current_pos = pos;
			break;
		}
	}
}

bool variable_index_iterator_is_done(
	const struct variable_index_iterator *iter)
{
	return iter->current_pos >= iter->variable_index->max_variables;
}

struct variable_info *variable_index_iterator_current(
	const struct variable_index_iterator *iter)
{
	struct variable_info *current = NULL;

	if (!variable_index_iterator_is_done(iter)) {

		current = &iter->variable_index->entries[iter->current_pos].info;
	}

	return current;
}

void variable_index_iterator_next(
	struct variable_index_iterator *iter)
{
	size_t next_pos = iter->current_pos;

	while (next_pos < iter->variable_index->max_variables) {

		++next_pos;
		if (iter->variable_index->entries[next_pos].in_use) break;
	}

	iter->current_pos = next_pos;
}

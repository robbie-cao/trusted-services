/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef VARIABLE_INDEX_H
#define VARIABLE_INDEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <protocols/common/efi/efi_status.h>
#include <protocols/service/smm_variable/smm_variable_proto.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Implementation limits
 */
#define VARIABLE_INDEX_MAX_NAME_SIZE		(32)

/**
 * \brief variable_info structure definition
 *
 * Holds information about a stored variable.
 */
struct variable_info
{
	EFI_GUID	guid;
	size_t		name_size;
	int16_t		name[VARIABLE_INDEX_MAX_NAME_SIZE];
	uint32_t	attributes;
	uint64_t	uid;
};

/**
 * \brief An entry in the index
 *
 * Represents a store variable in the variable index.
 */
struct variable_entry
{
	struct 	variable_info info;

	bool	in_use;
	bool	dirty;
};

/**
 * \brief variable_index structure definition
 *
 * Provides an index of stored variables to allow the uefi variable store
 * contents to be enumerated.
 */
struct variable_index
{
	size_t max_variables;
	struct variable_entry *entries;
};

/**
 * @brief      Initialises a variable_index
 *
 * @param[in]  context variable_index
 * @param[in]  max_variables The maximum number of stored variables
 *
 * @return     EFI_SUCCESS if initialized successfully
 */
efi_status_t variable_index_init(
	struct variable_index *context,
	size_t max_variables);

/**
 * @brief      De-initialises a variable_index
 *
 * @param[in]  context variable_index
 */
void variable_index_deinit(
	struct variable_index *context);

/**
 * @brief      Returns the maximum dump size
 *
 * For a given maximum index size, returns the size of the
 * buffer that is needed to hold all serialized variable_info
 * objects.
 *
 * @param[in]  context variable_index
 */
size_t variable_index_max_dump_size(
	struct variable_index *context);

/**
 * @brief      Find info about a variable
 *
 * @param[in]  context variable_index
 * @param[in]  guid The variable's guid
 * @param[in]  name_size The name parameter's size
 * @param[in]  name The variable's name
 *
 * @return     Pointer to variable_info or NULL
 */
const struct variable_info *variable_index_find(
	const struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name);

/**
 * @brief      Find the next variable in the index
 *
 * @param[in]  context variable_index
 * @param[in]  guid The variable's guid
 * @param[in]  name_size The name parameter's size
 * @param[in]  name The variable's name
 *
 * @return     Pointer to variable_info or NULL
 */
const struct variable_info *variable_index_find_next(
	const struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name);

/**
 * @brief      Add a new variable to the index
 *
 * @param[in]  context variable_index
 * @param[in]  guid The variable's guid
 * @param[in]  name_size The name parameter's size
 * @param[in]  name The variable's name
 * @param[in]  attributes The variable's attributes
 *
 * @return     Pointer to variable_info or NULL
 */
const struct variable_info *variable_index_add(
	struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name,
	uint32_t attributes);

/**
 * @brief      Remove a variable from the index
 *
 * Removes a variable from the index if it exists.
 *
 * @param[in]  context variable_index
 * @param[in]  guid The variable's guid
 * @param[in]  name_size The name parameter's size
 * @param[in]  name The variable's name
 */
void variable_index_remove(
	struct variable_index *context,
	const EFI_GUID *guid,
	size_t name_size,
	const int16_t *name);

/**
 * @brief      Update variable attributes
 *
 * @param[in]  context variable_index
 * @param[in]  info variable info
 * @param[in]  attributes The variable's attributes
 */
void variable_index_update_attributes(
	struct variable_index *context,
	const struct variable_info *info,
	uint32_t attributes);

/**
 * @brief      Dump the serialized index contents for persistent backup
 *
 * @param[in]  context variable_index
 * @param[in]  buffer_size Size of destination buffer
 * @param[in]  buffer Dump to this buffer
 * @param[out] data_len Length of serialized data
 *
 * @return     True if there is unsaved data
 */
bool variable_index_dump(
	struct variable_index *context,
	size_t buffer_size,
	uint8_t *buffer,
	size_t *data_len);

/**
 * @brief      Restore the serialized index contents
 *
 * Should be called straight after the variable index is initialized to
 * restore any NV variable info from persistent storage.
 *
 * @param[in]  context variable_index
 * @param[in]  data_len The length of the data to load
 * @param[in]  buffer Load from this buffer
 *
 * @return     Number of bytes loaded
 */
size_t variable_index_restore(
	const struct variable_index *context,
	size_t data_len,
	const uint8_t *buffer);


#ifdef __cplusplus
}
#endif

#endif /* VARIABLE_INDEX_H */

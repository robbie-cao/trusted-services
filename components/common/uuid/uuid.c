/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "uuid.h"
#include <string.h>
#include <ctype.h>

static uint8_t hex_to_nibble(char hex)
{
	uint8_t nibble = 0;

	if (hex >= '0' && hex <= '9') {
		nibble = hex - '0';
	}
	else {
		nibble = ((hex | 0x20) - 'a') + 10;
	}

	return nibble;
}

static uint8_t hex_to_byte(const char *hex)
{
	/* Takes a validated input and returns the byte value */
	uint8_t byte = hex_to_nibble(hex[0]) << 4;
	byte |= (hex_to_nibble(hex[1]) & 0x0f);
	return byte;
}

size_t uuid_is_valid(const char *canonical_form)
{
	size_t valid_chars = 0;
	size_t input_len = strlen(canonical_form);

	if (input_len >= UUID_CANONICAL_FORM_LEN) {

		size_t i;
		valid_chars = UUID_CANONICAL_FORM_LEN;

		for (i = 0; i < UUID_CANONICAL_FORM_LEN; ++i) {

			if (i == 8 || i == 13 || i == 18 || i == 23) {
				if (canonical_form[i] != '-') return 0;
			}
			else {
				if (!isxdigit((int)canonical_form[i])) return 0;
			}
		}
	}

	return valid_chars;
}

size_t uuid_parse_to_octets(const char *canonical_form, uint8_t *buf, size_t buf_size)
{
	size_t octet_index = 0;
	const char *pos;
	size_t valid_chars = uuid_is_valid(canonical_form);

	if ((buf_size < UUID_OCTETS_LEN) ||
		(valid_chars != UUID_CANONICAL_FORM_LEN)) {
		/* Invalid input */
		return 0;
	}

	/*
	 * UUID string has been validates as having the following form:
	 * xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
	 *     4      2    2   2       6
	 */
	pos = &canonical_form[0];
	while (octet_index < 4) {
		buf[octet_index++] = hex_to_byte(pos);
		pos += 2;
	}

	pos = &canonical_form[9];
	while (octet_index < 6) {
		buf[octet_index++] = hex_to_byte(pos);
		pos += 2;
	}

	pos = &canonical_form[14];
	while (octet_index < 8) {
		buf[octet_index++] = hex_to_byte(pos);
		pos += 2;
	}

	pos = &canonical_form[19];
	while (octet_index < 10) {
		buf[octet_index++] = hex_to_byte(pos);
		pos += 2;
	}

	pos = &canonical_form[24];
	while (octet_index < 16) {
		buf[octet_index++] = hex_to_byte(pos);
		pos += 2;
	}

	return valid_chars;
}

/*
 * The byte order is reversed for the first 4 bytes, then 2 bytes, then 2 bytes.
 * This is for compatibility with TF-A and OP-TEE where a binary uuid is represented as
 * an uint32_t, 2x uint16_t, then uint8_t array.
 */
void uuid_reverse_octets(const struct uuid_octets *standard_encoding,
	uint8_t *buf, size_t buf_size)
{
	if (buf_size >= UUID_OCTETS_LEN) {
		/* Reverse bytes in each section */
		buf[0] = standard_encoding->octets[3];
		buf[1] = standard_encoding->octets[2];
		buf[2] = standard_encoding->octets[1];
		buf[3] = standard_encoding->octets[0];

		buf[4] = standard_encoding->octets[5];
		buf[5] = standard_encoding->octets[4];

		buf[6] = standard_encoding->octets[7];
		buf[7] = standard_encoding->octets[6];

		buf[8] = standard_encoding->octets[8];
		buf[9] = standard_encoding->octets[9];

		buf[10] = standard_encoding->octets[10];
		buf[11] = standard_encoding->octets[11];
		buf[12] = standard_encoding->octets[12];
		buf[13] = standard_encoding->octets[13];
		buf[14] = standard_encoding->octets[14];
		buf[15] = standard_encoding->octets[15];
	}
}

size_t uuid_parse_to_octets_reversed(const char *canonical_form,
	uint8_t *buf, size_t buf_size)
{
	size_t valid_chars;
	struct uuid_octets standard_encoding;

	valid_chars = uuid_parse_to_octets(canonical_form,
		standard_encoding.octets, sizeof(standard_encoding.octets));

	if (valid_chars == UUID_CANONICAL_FORM_LEN) {

		uuid_reverse_octets(&standard_encoding, buf, buf_size);
	}

	return valid_chars;
}

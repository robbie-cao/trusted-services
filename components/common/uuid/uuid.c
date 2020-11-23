/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "uuid.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
                if (!isxdigit(canonical_form[i])) return 0;
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
        sscanf(pos, "%02hhX", &buf[octet_index++]);
        pos += 2;
    }

    pos = &canonical_form[9];
    while (octet_index < 6) {
        sscanf(pos, "%02hhX", &buf[octet_index++]);
        pos += 2;
    }

    pos = &canonical_form[14];
    while (octet_index < 8) {
        sscanf(pos, "%02hhX", &buf[octet_index++]);
        pos += 2;
    }

    pos = &canonical_form[19];
    while (octet_index < 10) {
        sscanf(pos, "%02hhX", &buf[octet_index++]);
        pos += 2;
    }

    pos = &canonical_form[24];
    while (octet_index < 16) {
        sscanf(pos, "%02hhX", &buf[octet_index++]);
        pos += 2;
    }

    return valid_chars;
}

/*
 * TODO: Temorary workaround for optee compatibility
 * The byte order is reversed for the first 4 bytes, then 2 bytes, then 2 bytes.
 * This is because the UUID type in OP-TEE consists of an uint32_t, 2x uint16_t,
 * then uint8_t array.
 */
size_t uuid_parse_to_octets_reversed(const char *canonical_form, uint8_t *buf, size_t buf_size)
{
    size_t valid_chars;
    uint8_t standard_octets[UUID_OCTETS_LEN];

    valid_chars = uuid_parse_to_octets(canonical_form, standard_octets, sizeof(standard_octets));

    if ((valid_chars == UUID_CANONICAL_FORM_LEN) && (buf_size >= UUID_OCTETS_LEN)) {
        /* Reverse bytes in each section */
        buf[0] = standard_octets[3];
        buf[1] = standard_octets[2];
        buf[2] = standard_octets[1];
        buf[3] = standard_octets[0];

        buf[4] = standard_octets[5];
        buf[5] = standard_octets[4];

        buf[6] = standard_octets[7];
        buf[7] = standard_octets[6];

        buf[8] = standard_octets[8];
        buf[9] = standard_octets[9];

        buf[10] = standard_octets[10];
        buf[11] = standard_octets[11];
        buf[12] = standard_octets[12];
        buf[13] = standard_octets[13];
        buf[14] = standard_octets[14];
        buf[15] = standard_octets[15];
    }

    return valid_chars;
}

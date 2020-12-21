/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PACKEDC_CRYPTO_PROVIDER_KEY_ATTRIBUTES_TRANSLATOR_H
#define PACKEDC_CRYPTO_PROVIDER_KEY_ATTRIBUTES_TRANSLATOR_H

#include <protocols/service/crypto/packed-c/key_attributes.h>
#include <psa/crypto.h>

void packedc_crypto_provider_translate_key_attributes(
    psa_key_attributes_t *psa_attributes,
    const struct ts_crypto_key_attributes *proto_attributes);

#endif /* PACKEDC_CRYPTO_PROVIDER_KEY_ATTRIBUTES_TRANSLATOR_H */
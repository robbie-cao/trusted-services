/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "packedc_key_attributes_translator.h"

void packedc_crypto_provider_translate_key_attributes(psa_key_attributes_t *psa_attributes,
    const struct ts_crypto_key_attributes *proto_attributes) {

    psa_set_key_type(psa_attributes, proto_attributes->type);
    psa_set_key_bits(psa_attributes, proto_attributes->key_bits);
    psa_set_key_lifetime(psa_attributes, proto_attributes->lifetime);

    if (proto_attributes->lifetime == PSA_KEY_LIFETIME_PERSISTENT) {

        psa_set_key_id(psa_attributes, proto_attributes->id);
    }

    psa_set_key_usage_flags(psa_attributes, proto_attributes->policy.usage);
    psa_set_key_algorithm(psa_attributes, proto_attributes->policy.alg);
}

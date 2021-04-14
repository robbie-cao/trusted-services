/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSA_CRYPTO_CLIENT_KEY_ATTRIBUTES_H
#define PSA_CRYPTO_CLIENT_KEY_ATTRIBUTES_H

#include <psa/crypto.h>
#include <protocols/service/crypto/packed-c/key_attributes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      Translate psa key attributes to packed-c serialization
 *
 * @param[out]  proto_attributes    The serialized key attributes
 * @param[in]   psa_attributes      psa key attributes from crypto api
 */
void psa_crypto_client_translate_key_attributes(
                    struct ts_crypto_key_attributes *proto_attributes,
                    const psa_key_attributes_t *psa_attributes);

#ifdef __cplusplus
}
#endif

#endif /* PSA_CRYPTO_CLIENT_KEY_ATTRIBUTES_H */

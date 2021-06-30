/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <psa/crypto.h>
#include <service/attestation/key_mngr/attest_key_mngr.h>
#include "local_attest_key_mngr.h"

/**
 * The singleton local_attest_key_mngr instance.
 */
static struct local_attest_key_mngr
{
    bool is_iak_open;
    psa_key_id_t iak_id;
    psa_key_handle_t iak_handle;
} instance;

/* Local defines */
#define IAK_KEY_BITS        (256)

/**
 * \brief Set the IAK key attributes
 *
 * \param[out] attribute   Key attributes object to set
 * \param[in] key_id       The IAK key id or zero for volatile key
 */
static void set_iak_attributes(psa_key_attributes_t *attributes, psa_key_id_t key_id)
{
    if (key_id)
        psa_set_key_id(attributes, key_id);
    else
        psa_set_key_lifetime(attributes, PSA_KEY_LIFETIME_VOLATILE);

    psa_set_key_usage_flags(attributes, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH);

    psa_set_key_algorithm(attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP256R1));
    psa_set_key_bits(attributes, IAK_KEY_BITS);
}

/**
 * \brief Generates the IAK
 *
 *  If an IAK hasn't been provisioned during manufacture, there is the
 *  option to generate a persistent IAK on first run.
 *
 * \param[in] key_id       The IAK key id or zero for volatile key
 * \param[out] iak_handle  The returned key handle
 *
 * \return Status
 */
static psa_status_t generate_iak(psa_key_id_t key_id, psa_key_handle_t *iak_handle)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    set_iak_attributes(&attributes, key_id);
    status = psa_generate_key(&attributes, iak_handle);

    psa_reset_key_attributes(&attributes);

    return status;
}

void local_attest_key_mngr_init(psa_key_id_t iak_id)
{
    instance.is_iak_open = false;
    instance.iak_id = iak_id;
    instance.iak_handle = -1;
}

void local_attest_key_mngr_deinit(void)
{
    if (instance.is_iak_open && !instance.iak_id) {

        /* Clean-up if IAK is volatile */
        psa_destroy_key(instance.iak_handle);
        instance.is_iak_open = false;
    }
}

psa_status_t attest_key_mngr_get_iak_handle(psa_key_handle_t *iak_handle)
{
    psa_status_t status = PSA_SUCCESS;

    if (!instance.is_iak_open) {

        if (instance.iak_id) {

            /* A valid key id has been specified so treat as a persistent key
             * that will normally already exist.
             */
            status = psa_open_key(instance.iak_id, &instance.iak_handle);

            if (status != PSA_SUCCESS) {

                /* First run and no key has been provisioned */
                status = generate_iak(instance.iak_id, &instance.iak_handle);
            }
        }
        else {

            /* An invalid key id has been specified which indicates that a
             * volatile key should be generated.  This is option is intended
             * for test purposes only.
             */
            status = generate_iak(instance.iak_id, &instance.iak_handle);
        }

        instance.is_iak_open = (status == PSA_SUCCESS);
    }

    *iak_handle = instance.iak_handle;
    return status;
}

psa_status_t attest_key_mngr_export_iak_public_key(uint8_t *data,
                                size_t data_size, size_t *data_length)
{
    psa_key_handle_t handle;
    psa_status_t status = attest_key_mngr_get_iak_handle(&handle);

    if (status == PSA_SUCCESS) {

        status = psa_export_public_key(handle, data, data_size, data_length);
    }

    return status;
}

size_t attest_key_mngr_max_iak_export_size(void)
{
    return PSA_KEY_EXPORT_MAX_SIZE(
        PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_CURVE_SECP256R1)),
            IAK_KEY_BITS);
}

size_t attest_key_mngr_max_iak_import_size(void)
{
    return PSA_BITS_TO_BYTES(IAK_KEY_BITS);
 }

psa_status_t attest_key_mngr_import_iak(const uint8_t *data, size_t data_length)
{
    psa_status_t status = PSA_ERROR_NOT_PERMITTED;

    if (!instance.is_iak_open) {

        psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
        set_iak_attributes(&attributes, instance.iak_id);

        if (instance.iak_id) {

            /* A valid key id has been specified so only allow import
             * if no persistent key for the key id exists.
             */
            if (psa_open_key(instance.iak_id, &instance.iak_handle) == PSA_ERROR_DOES_NOT_EXIST) {

                /* Allow persistent key to be provisioned */
                status = psa_import_key(&attributes, data, data_length, &instance.iak_handle);
            }
        }
        else {

            /* It's a volatile key so allow a once-per-boot import */
            status = psa_import_key(&attributes, data, data_length, &instance.iak_handle);
        }

        psa_reset_key_attributes(&attributes);

        instance.is_iak_open = (status == PSA_SUCCESS);
    }

    return status;
}

bool attest_key_mngr_iak_exists(void)
{
    if (!instance.is_iak_open && instance.iak_id) {

        /* A persistent key ID is specified so the key might
         * exist but has not been opened yet.
         */
        psa_status_t status = psa_open_key(instance.iak_id, &instance.iak_handle);
        instance.is_iak_open = (status == PSA_SUCCESS);
    }

    return instance.is_iak_open;
}

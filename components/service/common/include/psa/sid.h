/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __PSA_MANIFEST_SID_H__
#define __PSA_MANIFEST_SID_H__

#ifdef __cplusplus
extern "C" {
#endif

/******** TFM_SP_PS ********/
#define TFM_PROTECTED_STORAGE_SERVICE_SID                          (0x00000060U)
#define TFM_PROTECTED_STORAGE_SERVICE_VERSION                      (1U)
#define TFM_PROTECTED_STORAGE_SERVICE_HANDLE                       (0x40000101U)

/* Invalid UID */
#define TFM_PS_INVALID_UID 0

/* PS / ITS message types that distinguish PS services. */
#define TFM_PS_ITS_SET                1001
#define TFM_PS_ITS_GET                1002
#define TFM_PS_ITS_GET_INFO           1003
#define TFM_PS_ITS_REMOVE             1004
#define TFM_PS_ITS_GET_SUPPORT        1005

/******** TFM_SP_ITS ********/
#define TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_SID                   (0x00000070U)
#define TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_VERSION               (1U)
#define TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE                (0x40000102U)

/******** TFM_SP_CRYPTO ********/
#define TFM_CRYPTO_SID                                             (0x00000080U)
#define TFM_CRYPTO_VERSION                                         (1U)
#define TFM_CRYPTO_HANDLE                                          (0x40000100U)

/**
 * \brief Define a progressive numerical value for each SID which can be used
 *        when dispatching the requests to the service
 */
enum {
    TFM_CRYPTO_GET_KEY_ATTRIBUTES_SID = (0u),
    TFM_CRYPTO_RESET_KEY_ATTRIBUTES_SID,
    TFM_CRYPTO_OPEN_KEY_SID,
    TFM_CRYPTO_CLOSE_KEY_SID,
    TFM_CRYPTO_IMPORT_KEY_SID,
    TFM_CRYPTO_DESTROY_KEY_SID,
    TFM_CRYPTO_EXPORT_KEY_SID,
    TFM_CRYPTO_EXPORT_PUBLIC_KEY_SID,
    TFM_CRYPTO_PURGE_KEY_SID,
    TFM_CRYPTO_COPY_KEY_SID,
    TFM_CRYPTO_HASH_COMPUTE_SID,
    TFM_CRYPTO_HASH_COMPARE_SID,
    TFM_CRYPTO_HASH_SETUP_SID,
    TFM_CRYPTO_HASH_UPDATE_SID,
    TFM_CRYPTO_HASH_FINISH_SID,
    TFM_CRYPTO_HASH_VERIFY_SID,
    TFM_CRYPTO_HASH_ABORT_SID,
    TFM_CRYPTO_HASH_CLONE_SID,
    TFM_CRYPTO_MAC_COMPUTE_SID,
    TFM_CRYPTO_MAC_VERIFY_SID,
    TFM_CRYPTO_MAC_SIGN_SETUP_SID,
    TFM_CRYPTO_MAC_VERIFY_SETUP_SID,
    TFM_CRYPTO_MAC_UPDATE_SID,
    TFM_CRYPTO_MAC_SIGN_FINISH_SID,
    TFM_CRYPTO_MAC_VERIFY_FINISH_SID,
    TFM_CRYPTO_MAC_ABORT_SID,
    TFM_CRYPTO_CIPHER_ENCRYPT_SID,
    TFM_CRYPTO_CIPHER_DECRYPT_SID,
    TFM_CRYPTO_CIPHER_ENCRYPT_SETUP_SID,
    TFM_CRYPTO_CIPHER_DECRYPT_SETUP_SID,
    TFM_CRYPTO_CIPHER_GENERATE_IV_SID,
    TFM_CRYPTO_CIPHER_SET_IV_SID,
    TFM_CRYPTO_CIPHER_UPDATE_SID,
    TFM_CRYPTO_CIPHER_FINISH_SID,
    TFM_CRYPTO_CIPHER_ABORT_SID,
    TFM_CRYPTO_AEAD_ENCRYPT_SID,
    TFM_CRYPTO_AEAD_DECRYPT_SID,
    TFM_CRYPTO_AEAD_ENCRYPT_SETUP_SID,
    TFM_CRYPTO_AEAD_DECRYPT_SETUP_SID,
    TFM_CRYPTO_AEAD_GENERATE_NONCE_SID,
    TFM_CRYPTO_AEAD_SET_NONCE_SID,
    TFM_CRYPTO_AEAD_SET_LENGTHS_SID,
    TFM_CRYPTO_AEAD_UPDATE_AD_SID,
    TFM_CRYPTO_AEAD_UPDATE_SID,
    TFM_CRYPTO_AEAD_FINISH_SID,
    TFM_CRYPTO_AEAD_VERIFY_SID,
    TFM_CRYPTO_AEAD_ABORT_SID,
    TFM_CRYPTO_SIGN_MESSAGE_SID,
    TFM_CRYPTO_VERIFY_MESSAGE_SID,
    TFM_CRYPTO_SIGN_HASH_SID,
    TFM_CRYPTO_VERIFY_HASH_SID,
    TFM_CRYPTO_ASYMMETRIC_ENCRYPT_SID,
    TFM_CRYPTO_ASYMMETRIC_DECRYPT_SID,
    TFM_CRYPTO_KEY_DERIVATION_SETUP_SID,
    TFM_CRYPTO_KEY_DERIVATION_GET_CAPACITY_SID,
    TFM_CRYPTO_KEY_DERIVATION_SET_CAPACITY_SID,
    TFM_CRYPTO_KEY_DERIVATION_INPUT_BYTES_SID,
    TFM_CRYPTO_KEY_DERIVATION_INPUT_KEY_SID,
    TFM_CRYPTO_KEY_DERIVATION_KEY_AGREEMENT_SID,
    TFM_CRYPTO_KEY_DERIVATION_OUTPUT_BYTES_SID,
    TFM_CRYPTO_KEY_DERIVATION_OUTPUT_KEY_SID,
    TFM_CRYPTO_KEY_DERIVATION_ABORT_SID,
    TFM_CRYPTO_RAW_KEY_AGREEMENT_SID,
    TFM_CRYPTO_GENERATE_RANDOM_SID,
    TFM_CRYPTO_GENERATE_KEY_SID,
    TFM_CRYPTO_SID_MAX,
};

/******** TFM_SP_PLATFORM ********/
#define TFM_SP_PLATFORM_SYSTEM_RESET_SID                           (0x00000040U)
#define TFM_SP_PLATFORM_SYSTEM_RESET_VERSION                       (1U)
#define TFM_SP_PLATFORM_IOCTL_SID                                  (0x00000041U)
#define TFM_SP_PLATFORM_IOCTL_VERSION                              (1U)
#define TFM_SP_PLATFORM_NV_COUNTER_SID                             (0x00000042U)
#define TFM_SP_PLATFORM_NV_COUNTER_VERSION                         (1U)

/******** TFM_SP_INITIAL_ATTESTATION ********/
#define TFM_ATTESTATION_SERVICE_SID                                (0x00000020U)
#define TFM_ATTESTATION_SERVICE_VERSION                            (1U)
#define TFM_ATTESTATION_SERVICE_HANDLE                             (0x40000103U)

/* Initial Attestation message types that distinguish Attest services. */
#define TFM_ATTEST_GET_TOKEN       1001
#define TFM_ATTEST_GET_TOKEN_SIZE  1002

/******** TFM_SP_FWU ********/
#define TFM_FWU_WRITE_SID                                          (0x000000A0U)
#define TFM_FWU_WRITE_VERSION                                      (1U)
#define TFM_FWU_INSTALL_SID                                        (0x000000A1U)
#define TFM_FWU_INSTALL_VERSION                                    (1U)
#define TFM_FWU_ABORT_SID                                          (0x000000A2U)
#define TFM_FWU_ABORT_VERSION                                      (1U)
#define TFM_FWU_QUERY_SID                                          (0x000000A3U)
#define TFM_FWU_QUERY_VERSION                                      (1U)
#define TFM_FWU_REQUEST_REBOOT_SID                                 (0x000000A4U)
#define TFM_FWU_REQUEST_REBOOT_VERSION                             (1U)
#define TFM_FWU_ACCEPT_SID                                         (0x000000A5U)
#define TFM_FWU_ACCEPT_VERSION                                     (1U)

#ifdef __cplusplus
}
#endif

#endif /* __PSA_MANIFEST_SID_H__ */

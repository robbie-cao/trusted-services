/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <mbedtls/build_info.h>
#include <mbedtls/pkcs7.h>
#include <mbedtls/x509_crt.h>
#include <stdint.h>

int verify_pkcs7_signature(const uint8_t *signature_cert, uint64_t signature_cert_len,
			   const uint8_t *hash, uint64_t hash_len, const uint8_t *public_key_cert,
			   uint64_t public_key_cert_len)
{
	int mbedtls_status = MBEDTLS_ERR_PKCS7_VERIFY_FAIL;

	/* Parse the public key certificate */
	mbedtls_x509_crt signer_certificate;

	mbedtls_x509_crt_init(&signer_certificate);

	mbedtls_status = mbedtls_x509_crt_parse_der(&signer_certificate, public_key_cert,
						    public_key_cert_len);

	if (mbedtls_status == 0) {
		/* Parse the PKCS#7 DER encoded signature block */
		mbedtls_pkcs7 pkcs7_structure;

		mbedtls_pkcs7_init(&pkcs7_structure);

		mbedtls_status = mbedtls_pkcs7_parse_der(&pkcs7_structure, signature_cert,
							 signature_cert_len);

		if (mbedtls_status == MBEDTLS_PKCS7_SIGNED_DATA) {
			/* Verify hash against signed hash */
			mbedtls_status = mbedtls_pkcs7_signed_hash_verify(
				&pkcs7_structure, &signer_certificate, hash, hash_len);
		}

		mbedtls_pkcs7_free(&pkcs7_structure);
	}

	mbedtls_x509_crt_free(&signer_certificate);

	return mbedtls_status;
}

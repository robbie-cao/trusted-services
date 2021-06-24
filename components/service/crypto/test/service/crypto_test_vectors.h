/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdint>
#include <vector>

/*
 * A selection of test vectors in the form of blocks of plaintext,
 * ciphertext, expected hashes etc.
 */
class crypto_test_vectors
{
public:

	static void plaintext_1_len_610(std::vector<uint8_t> &plaintext);
	static void sha256_1(std::vector<uint8_t> &hash);

};

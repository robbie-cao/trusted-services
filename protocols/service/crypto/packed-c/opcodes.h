/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TS_CRYPTO_OPCODES_H
#define TS_CRYPTO_OPCODES_H

/* C/C++ definition of crypto service opcodes
 */
#define TS_CRYPTO_OPCODE_NOP                    (0x0000)
#define TS_CRYPTO_OPCODE_GENERATE_KEY           (0x0101)
#define TS_CRYPTO_OPCODE_DESTROY_KEY            (0x0102)
#define TS_CRYPTO_OPCODE_OPEN_KEY               (0x0103)
#define TS_CRYPTO_OPCODE_CLOSE_KEY              (0x0104)
#define TS_CRYPTO_OPCODE_EXPORT_KEY             (0x0105)
#define TS_CRYPTO_OPCODE_EXPORT_PUBLIC_KEY      (0x0106)
#define TS_CRYPTO_OPCODE_IMPORT_KEY             (0x0107)
#define TS_CRYPTO_OPCODE_SIGN_HASH              (0x0108)
#define TS_CRYPTO_OPCODE_VERIFY_HASH            (0x0109)
#define TS_CRYPTO_OPCODE_ASYMMETRIC_DECRYPT     (0x010a)
#define TS_CRYPTO_OPCODE_ASYMMETRIC_ENCRYPT     (0x010b)
#define TS_CRYPTO_OPCODE_GENERATE_RANDOM        (0x010c)

#endif /* TS_CRYPTO_OPCODES_H */

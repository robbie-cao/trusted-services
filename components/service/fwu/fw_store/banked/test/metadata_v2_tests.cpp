/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstddef>
#include <protocols/service/fwu/packed-c/metadata_v2.h>
#include <CppUTest/TestHarness.h>

/* Tests check FWU metadata protocol definitions are aligned to the FWU-A
 * specification.
*/
TEST_GROUP(FwuMetadataV2Tests)
{

};

TEST(FwuMetadataV2Tests, checkHeaderStructure)
{
	/* Check header structure offsets against offsets from spec */
	UNSIGNED_LONGS_EQUAL(0x00, offsetof(fwu_metadata, crc_32));
	UNSIGNED_LONGS_EQUAL(0x04, offsetof(fwu_metadata, version));
	UNSIGNED_LONGS_EQUAL(0x08, offsetof(fwu_metadata, metadata_size));
	UNSIGNED_LONGS_EQUAL(0x0c, offsetof(fwu_metadata, header_size));
	UNSIGNED_LONGS_EQUAL(0x0e, offsetof(fwu_metadata, active_index));  /* Note spec incorrectly says 0x0d - fed back */
	UNSIGNED_LONGS_EQUAL(0x0f, offsetof(fwu_metadata, previous_active_index));
	UNSIGNED_LONGS_EQUAL(0x10, offsetof(fwu_metadata, bank_state));

	/* Check header size aligns with expected offset of optional fw_store_desc */
	UNSIGNED_LONGS_EQUAL(0x14, sizeof(struct fwu_metadata));  /* Note spec incorrectly says 0x11 - fed back */
}

TEST(FwuMetadataV2Tests, checkFwStoreDescStructure)
{
	/* Check fw_store_desc structure offsets against offsets from spec */
	UNSIGNED_LONGS_EQUAL(0x00, offsetof(fwu_fw_store_desc, num_banks));
	UNSIGNED_LONGS_EQUAL(0x01, offsetof(fwu_fw_store_desc, num_images));
	UNSIGNED_LONGS_EQUAL(0x03, offsetof(fwu_fw_store_desc, img_entry_size));
	UNSIGNED_LONGS_EQUAL(0x05, offsetof(fwu_fw_store_desc, bank_entry_size));
	UNSIGNED_LONGS_EQUAL(0x07, offsetof(fwu_fw_store_desc, img_entry));
}

TEST(FwuMetadataV2Tests, checkImgEntryStructure)
{
	/* Check img_entry structure offsets against offsets from spec */
	UNSIGNED_LONGS_EQUAL(0x00, offsetof(fwu_image_entry, img_type_uuid));
	UNSIGNED_LONGS_EQUAL(0x10, offsetof(fwu_image_entry, location_uuid));
	UNSIGNED_LONGS_EQUAL(0x20, offsetof(fwu_image_entry, img_bank_info));
}

TEST(FwuMetadataV2Tests, checkImgBankInfoStructure)
{
	/* Check img_entry structure offsets against offsets from spec */
	UNSIGNED_LONGS_EQUAL(0x00, offsetof(fwu_img_bank_info, img_uuid));
	UNSIGNED_LONGS_EQUAL(0x10, offsetof(fwu_img_bank_info, accepted));
	UNSIGNED_LONGS_EQUAL(0x14, offsetof(fwu_img_bank_info, reserved));

	/* Check img_bank_info size aligns with specified value */
	UNSIGNED_LONGS_EQUAL(0x18, sizeof(struct fwu_img_bank_info));
}
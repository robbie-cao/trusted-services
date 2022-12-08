/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <vector>
#include <protocols/service/fwu/packed-c/status.h>
#include <CppUTest/TestHarness.h>
#include <service/fwu/test/fwu_dut_factory/fwu_dut_factory.h>
#include <service/fwu/test/fwu_dut/fwu_dut.h>

/*
 * Tests that perform a range of normal update scenarios
 */
TEST_GROUP(FwuUpdateScenarioTests)
{
	void setup()
	{
		m_dut = NULL;
		m_fwu_client = NULL;
		m_metadata_checker = NULL;
	}

	void teardown()
	{
		delete m_metadata_checker;
		m_metadata_checker = NULL;

		delete m_fwu_client;
		m_fwu_client = NULL;

		delete m_dut;
		m_dut = NULL;
	}

	fwu_dut *m_dut;
	metadata_checker *m_metadata_checker;
	fwu_client *m_fwu_client;
};

TEST(FwuUpdateScenarioTests, wholeFirmwareUpdateFlow)
{
	int status = 0;
	struct uuid_octets uuid;
	uint32_t stream_handle = 0;

	/* Performs an update flow where firmware for a device with a
	 * single location is updated using a whole firmware image.
	 */
	m_dut = fwu_dut_factory::create(1, false);
	m_fwu_client = m_dut->create_fwu_client();
	m_metadata_checker = m_dut->create_metadata_checker();

	m_dut->boot();

	/* Check assumptions about the first boot. This represents the fresh out
	 * of the factory state of the device where no updates have ever been
	 * performed,
	 */
	struct boot_info boot_info = m_dut->get_boot_info();
	UNSIGNED_LONGS_EQUAL(boot_info.boot_index, boot_info.active_index);
	m_metadata_checker->check_regular(boot_info.boot_index);

	/* Note the pre-update bank index */
	unsigned int pre_update_bank_index = boot_info.boot_index;

	/* Perform staging steps where a single image is installed */
	status = m_fwu_client->begin_staging();
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);
	m_metadata_checker->check_ready_for_staging(boot_info.boot_index);

	m_dut->whole_volume_image_type_uuid(0, &uuid);
	status = m_fwu_client->open(&uuid, &stream_handle);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	std::vector<uint8_t> image_data;
	m_dut->generate_image_data(&image_data);

	status = m_fwu_client->write_stream(stream_handle,
		reinterpret_cast<const uint8_t *>(image_data.data()), image_data.size());
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	status = m_fwu_client->commit(stream_handle, false);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	status = m_fwu_client->end_staging();
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);
	m_metadata_checker->check_ready_to_activate(boot_info.boot_index);

	/* Reboot to activate the update */
	m_dut->shutdown();
	m_dut->boot();

	/* Check the metadata after the reboot to activate the update */
	boot_info = m_dut->get_boot_info();
	m_metadata_checker->check_trial(boot_info.boot_index);

	/* Accept the update. Because the DUT was configured with a single updatable
	 * image, only a single image needs to be accepted to complete the update
	 * transaction.
	 */
	m_dut->whole_volume_image_type_uuid(0, &uuid);
	status = m_fwu_client->accept(&uuid);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	/* The FWU metadata should now reflect the post update state */
	m_metadata_checker->check_regular(boot_info.boot_index);

	/* The update agent should also have transitioned from the trial state.
	 * Confirm this trying an operation that's only permitted in the trial state.
	 */
	status = m_fwu_client->accept(&uuid);
	LONGS_EQUAL(FWU_STATUS_DENIED, status);

	/* Reboot and expect the update to be active */
	m_dut->shutdown();
	m_dut->boot();

	boot_info = m_dut->get_boot_info();
	m_metadata_checker->check_regular(boot_info.boot_index);
	CHECK_TRUE(boot_info.boot_index != pre_update_bank_index);
}

TEST(FwuUpdateScenarioTests, partialFirmwareUpdateFlow)
{
	int status = 0;
	struct uuid_octets uuid;
	uint32_t stream_handle = 0;

	/* Performs an update flow where firmware for a device with
	 * multiple firmware locations is updated with a partial update
	 * that only updates a subset of locations. The expectation is
	 * that locations that have not been updated will use the current
	 * version of firmware for the location.
	 */
	m_dut = fwu_dut_factory::create(3, true);
	m_fwu_client = m_dut->create_fwu_client();
	m_metadata_checker = m_dut->create_metadata_checker();

	m_dut->boot();

	/* Check state on first boot */
	struct boot_info boot_info = m_dut->get_boot_info();
	UNSIGNED_LONGS_EQUAL(boot_info.boot_index, boot_info.active_index);
	m_metadata_checker->check_regular(boot_info.boot_index);

	/* Note the pre-update bank index */
	unsigned int pre_update_bank_index = boot_info.boot_index;

	/* Perform staging steps where multiple images are installed */
	std::vector<uint8_t> image_data;
	status = m_fwu_client->begin_staging();
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);
	m_metadata_checker->check_ready_for_staging(boot_info.boot_index);

	/* Install whole image for location 0 */
	m_dut->whole_volume_image_type_uuid(0, &uuid);
	status = m_fwu_client->open(&uuid, &stream_handle);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	m_dut->generate_image_data(&image_data);

	status = m_fwu_client->write_stream(stream_handle,
		reinterpret_cast<const uint8_t *>(image_data.data()), image_data.size());
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	/* Accept on commit. No need to accept during trial */
	status = m_fwu_client->commit(stream_handle, true);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	/* Install whole image for location 1 */
	m_dut->whole_volume_image_type_uuid(1, &uuid);
	status = m_fwu_client->open(&uuid, &stream_handle);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	m_dut->generate_image_data(&image_data);

	status = m_fwu_client->write_stream(stream_handle,
		reinterpret_cast<const uint8_t *>(image_data.data()), image_data.size());
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	status = m_fwu_client->commit(stream_handle, false);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	/* Don't update location 2 */

	/* End staging */
	status = m_fwu_client->end_staging();
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);
	m_metadata_checker->check_ready_to_activate(boot_info.boot_index);

	/* Reboot to activate the update */
	m_dut->shutdown();
	m_dut->boot();

	/* Check the metadata after the reboot to activate the update */
	boot_info = m_dut->get_boot_info();
	m_metadata_checker->check_trial(boot_info.boot_index);

	/* Accept the update. Image for location 0 was accepted on commit.
	 * Image 2 wasn't updated so it should have been copied as accepted
	 * so only image 1 needs to be accepted.
	 */
	m_dut->whole_volume_image_type_uuid(1, &uuid);
	status = m_fwu_client->accept(&uuid);
	LONGS_EQUAL(FWU_STATUS_SUCCESS, status);

	/* The FWU metadata should now reflect the post update state */
	m_metadata_checker->check_regular(boot_info.boot_index);

	/* The update agent should also have transitioned from the trial state. */
	status = m_fwu_client->accept(&uuid);
	LONGS_EQUAL(FWU_STATUS_DENIED, status);

	/* Reboot and expect the update to be active */
	m_dut->shutdown();
	m_dut->boot();

	boot_info = m_dut->get_boot_info();
	m_metadata_checker->check_regular(boot_info.boot_index);
	CHECK_TRUE(boot_info.boot_index != pre_update_bank_index);
}

/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string>
#include <vector>
#include <string.h>
#include <CppUTest/TestHarness.h>
#include <service/smm_variable/backend/uefi_variable_store.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>

TEST_GROUP(UefiVariableStoreTests)
{
	void setup()
	{
		m_persistent_backend = mock_store_init(&m_persistent_store);
		m_volatile_backend = mock_store_init(&m_volatile_store);

		efi_status_t status = uefi_variable_store_init(
			&m_uefi_variable_store,
			OWNER_ID,
			MAX_VARIABLES,
			m_persistent_backend,
			m_volatile_backend);

		UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

		setup_common_guid();
	}

	void teardown()
	{
		uefi_variable_store_deinit(&m_uefi_variable_store);
	}

	void setup_common_guid()
	{
		m_common_guid.Data1 = 0x12341234;
		m_common_guid.Data2 = 0x1234;
		m_common_guid.Data3 = 0x1234;
		m_common_guid.Data4[0] = 0x00;
		m_common_guid.Data4[1] = 0x01;
		m_common_guid.Data4[2] = 0x02;
		m_common_guid.Data4[3] = 0x03;
		m_common_guid.Data4[4] = 0x04;
		m_common_guid.Data4[5] = 0x05;
		m_common_guid.Data4[6] = 0x06;
		m_common_guid.Data4[7] = 0x07;
	}

	std::vector<int16_t> to_variable_name(const std::wstring &string)
	{
		std::vector<int16_t> var_name;

		for (size_t i = 0; i < string.size(); i++) {

			var_name.push_back((int16_t)string[i]);
		}

		/* Add mandatory null terminator */
		var_name.push_back(0);

		return var_name;
	}

	bool compare_variable_name(
		const std::wstring &expected,
		const int16_t *name,
		size_t name_size) {

		bool is_equal = (expected.size() + 1 <= name_size / sizeof(int16_t));

		for (size_t i = 0; is_equal && i < expected.size(); i++) {

			if (name[i] != (int16_t)expected[i]) {

				is_equal = false;
				break;
			}
		}

		return is_equal;
	}

	efi_status_t set_variable(
		const std::wstring &name,
		const std::string &data,
		uint32_t attributes)
	{
		std::vector<int16_t> var_name = to_variable_name(name);
		size_t name_size = var_name.size() * sizeof(int16_t);
		size_t data_size = data.size();
		uint8_t msg_buffer[SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_SIZE(name_size, data_size)];

		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *access_variable =
			(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)msg_buffer;

		access_variable->Guid = m_common_guid;
		access_variable->Attributes = attributes;

		access_variable->NameSize = name_size;
		memcpy(access_variable->Name, var_name.data(), name_size);

		access_variable->DataSize = data_size;
		memcpy(&msg_buffer[SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(access_variable)],
			data.c_str(), data_size);

		efi_status_t status = uefi_variable_store_set_variable(
			&m_uefi_variable_store,
			access_variable);

		return status;
	}

	efi_status_t get_variable(
		const std::wstring &name,
		std::string &data)
	{
		std::vector<int16_t> var_name = to_variable_name(name);
		size_t name_size = var_name.size() * sizeof(int16_t);
		size_t total_size = 0;
		uint8_t msg_buffer[VARIABLE_BUFFER_SIZE];

		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *access_variable =
			(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE*)msg_buffer;

		access_variable->Guid = m_common_guid;
		access_variable->Attributes = 0;

		access_variable->NameSize = name_size;
		memcpy(access_variable->Name, var_name.data(), name_size);

		access_variable->DataSize = 0;

		efi_status_t status = uefi_variable_store_get_variable(
			&m_uefi_variable_store,
			access_variable,
			VARIABLE_BUFFER_SIZE -
				SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(access_variable),
			&total_size);

		if (status == EFI_SUCCESS) {

			const char *data_start = (const char*)(msg_buffer +
				SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_DATA_OFFSET(access_variable));

			data = std::string(data_start, access_variable->DataSize);
		}

		return status;
	}

	efi_status_t set_check_var_property(
		const std::wstring &name,
		const VAR_CHECK_VARIABLE_PROPERTY &check_property)
	{
		std::vector<int16_t> var_name = to_variable_name(name);
		size_t name_size = var_name.size() * sizeof(int16_t);
		uint8_t msg_buffer[SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY_SIZE(name_size)];

		SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY *check_var =
			(SMM_VARIABLE_COMMUNICATE_VAR_CHECK_VARIABLE_PROPERTY*)msg_buffer;

		check_var->Guid = m_common_guid;
		check_var->NameSize = name_size;
		memcpy(check_var->Name, var_name.data(), name_size);

		check_var->VariableProperty = check_property;

		efi_status_t status = uefi_variable_store_set_var_check_property(
			&m_uefi_variable_store,
			check_var);

		return status;
	}

	void zap_stored_variable(
		const std::wstring &name)
	{
		std::vector<int16_t> var_name = to_variable_name(name);
		size_t name_size = var_name.size() * sizeof(int16_t);

		/* Create the condition where a variable is indexed but
		 * there is no corresponding stored object.
		 */
		struct variable_index *variable_index = &m_uefi_variable_store.variable_index;

		const struct variable_info *info = variable_index_find(
			variable_index,
			&m_common_guid,
			name_size,
			var_name.data());

		if (info && (info->metadata.attributes & EFI_VARIABLE_NON_VOLATILE)) {

			struct storage_backend *storage_backend = m_uefi_variable_store.persistent_store;

			storage_backend->interface->remove(
				storage_backend->context,
				OWNER_ID,
				info->metadata.uid);
		}
	}

	void power_cycle()
	{
		/* Simulate a power-cycle */
		uefi_variable_store_deinit(&m_uefi_variable_store);

		/* Lose volatile store contents */
		mock_store_reset(&m_volatile_store);

		efi_status_t status = uefi_variable_store_init(
			&m_uefi_variable_store,
			OWNER_ID,
			MAX_VARIABLES,
			m_persistent_backend,
			m_volatile_backend);

		UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	}

	static const size_t MAX_VARIABLES = 10;
	static const uint32_t OWNER_ID = 100;
	static const size_t VARIABLE_BUFFER_SIZE = 1024;

	struct uefi_variable_store m_uefi_variable_store;
	struct mock_store m_persistent_store;
	struct mock_store m_volatile_store;
	struct storage_backend *m_persistent_backend;
	struct storage_backend *m_volatile_backend;
	EFI_GUID m_common_guid;
};

TEST(UefiVariableStoreTests, setGetRoundtrip)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name = L"test_variable";
	std::string input_data = "quick brown fox";
	std::string output_data;

	status = set_variable(var_name, input_data, 0);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Expect got variable data to be the same as the set value */
	UNSIGNED_LONGS_EQUAL(input_data.size(), output_data.size());
	LONGS_EQUAL(0, input_data.compare(output_data));
}

TEST(UefiVariableStoreTests, persistentSetGet)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name = L"test_variable";
	std::string input_data = "quick brown fox";
	std::string output_data;

	status = set_variable(var_name, input_data, EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Expect got variable data to be the same as the set value */
	UNSIGNED_LONGS_EQUAL(input_data.size(), output_data.size());
	LONGS_EQUAL(0, input_data.compare(output_data));

	/* Expect the variable to survive a power cycle */
	power_cycle();

	output_data = std::string();
	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Still expect got variable data to be the same as the set value */
	UNSIGNED_LONGS_EQUAL(input_data.size(), output_data.size());
	LONGS_EQUAL(0, input_data.compare(output_data));
}

TEST(UefiVariableStoreTests, removeVolatile)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name = L"rm_volatile_variable";
	std::string input_data = "quick brown fox";
	std::string output_data;

	status = set_variable(var_name, input_data, 0);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Remove by setting with zero data length */
	status = set_variable(var_name, std::string(), 0);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Expect variable to no loger exist */
	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_NOT_FOUND, status);
}

TEST(UefiVariableStoreTests, removePersistent)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name = L"rm_nv_variable";
	std::string input_data = "quick brown fox";
	std::string output_data;

	status = set_variable(var_name, input_data, EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Remove by setting with zero data length */
	status = set_variable(var_name, std::string(), 0);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Expect variable to no loger exist */
	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_NOT_FOUND, status);
}

TEST(UefiVariableStoreTests, bootServiceAccess)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name  = L"test_variable";
	std::string input_data = "a variable with access restricted to boot";
	std::string output_data;

	status = set_variable(
		var_name,
		input_data,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* 'Reboot' */
	power_cycle();

	/* Expect access to be permitted */
	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	UNSIGNED_LONGS_EQUAL(input_data.size(), output_data.size());
	LONGS_EQUAL(0, input_data.compare(output_data));

	/* End of boot phase */
	status = uefi_variable_store_exit_boot_service(&m_uefi_variable_store);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Expect access to be blocked */
	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_ACCESS_DENIED, status);
}

TEST(UefiVariableStoreTests, runtimeAccess)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name  = L"test_variable";
	std::string input_data = "a variable with access restricted to runtime";
	std::string output_data;

	status = set_variable(
		var_name,
		input_data,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* 'Reboot' */
	power_cycle();

	status = get_variable(var_name, output_data);

	UNSIGNED_LONGS_EQUAL(EFI_ACCESS_DENIED, status);
	/* End of boot phase */
	status = uefi_variable_store_exit_boot_service(&m_uefi_variable_store);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Expect access to be permitted */
	status = get_variable(var_name, output_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	UNSIGNED_LONGS_EQUAL(input_data.size(), output_data.size());
	LONGS_EQUAL(0, input_data.compare(output_data));
}

TEST(UefiVariableStoreTests, enumerateStoreContents)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name_1 = L"test_variable_1";
	std::wstring var_name_2 = L"test_variable_2";
	std::wstring var_name_3 = L"test_variable_3";
	std::string input_data = "blah blah";

	/* Add some variables - a mixture of NV and volatile */
	status = set_variable(
		var_name_1,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = set_variable(
		var_name_2,
		input_data,
		0);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = set_variable(
		var_name_3,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Prepare to enumerate */
	uint8_t msg_buffer[VARIABLE_BUFFER_SIZE];
	SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME *next_name =
		(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME*)msg_buffer;
	size_t max_name_len = VARIABLE_BUFFER_SIZE -
		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET;

	/* Enumerate store contents */
	size_t total_len = 0;
	next_name->NameSize = sizeof(int16_t);
	next_name->Name[0] = 0;

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	CHECK_TRUE(compare_variable_name(var_name_1, next_name->Name, next_name->NameSize));

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	CHECK_TRUE(compare_variable_name(var_name_2, next_name->Name, next_name->NameSize));

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	CHECK_TRUE(compare_variable_name(var_name_3, next_name->Name, next_name->NameSize));

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_NOT_FOUND, status);

	power_cycle();

	/* Enumerate again - should be left with just NV variables.
	 * Use a different but equally valid null name.
	 */
	next_name->NameSize = 10 * sizeof(int16_t);
	memset(next_name->Name, 0, next_name->NameSize);

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	CHECK_TRUE(compare_variable_name(var_name_1, next_name->Name, next_name->NameSize));

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, status);
	CHECK_TRUE(compare_variable_name(var_name_3, next_name->Name, next_name->NameSize));

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_NOT_FOUND, status);
}

TEST(UefiVariableStoreTests, failedNvSet)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name_1 = L"test_variable_1";
	std::wstring var_name_2 = L"test_variable_2";
	std::wstring var_name_3 = L"test_variable_3";
	std::string input_data = "blah blah";

	/* Add some variables - a mixture of NV and volatile */
	status = set_variable(
		var_name_1,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = set_variable(
		var_name_2,
		input_data,
		0);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	status = set_variable(
		var_name_3,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Simulate a power failure which resulted in the
	 * variable index being written but not the corresponding
	 * data.
	 */
	zap_stored_variable(var_name_3);
	power_cycle();

	/* After the power cycle, we expect the volatile variable
	 * to have gone and for the index to have been cleaned up
	 * for the failed variable 3.
	 */
	uint8_t msg_buffer[VARIABLE_BUFFER_SIZE];
	SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME *next_name =
		(SMM_VARIABLE_COMMUNICATE_GET_NEXT_VARIABLE_NAME*)msg_buffer;
	size_t max_name_len = VARIABLE_BUFFER_SIZE -
		SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE_NAME_OFFSET;

	/* Enumerate store contents */
	size_t total_len = 0;
	next_name->NameSize = sizeof(int16_t);
	next_name->Name[0] = 0;

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);
	CHECK_TRUE(compare_variable_name(var_name_1, next_name->Name, next_name->NameSize));

	status = uefi_variable_store_get_next_variable_name(
		&m_uefi_variable_store,
		next_name,
		max_name_len,
		&total_len);
	UNSIGNED_LONGS_EQUAL(EFI_NOT_FOUND, status);
}

TEST(UefiVariableStoreTests, readOnlycheck)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name_1 = L"test_variable_1";
	std::string input_data = "blah blah";

	/* Add a variable */
	status = set_variable(
		var_name_1,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Apply a check to constrain to Read Only */
	VAR_CHECK_VARIABLE_PROPERTY check_property;
	check_property.Revision = VAR_CHECK_VARIABLE_PROPERTY_REVISION;
	check_property.Attributes = 0;
	check_property.Property = VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
	check_property.MinSize = 0;
	check_property.MaxSize = 100;

	status = set_check_var_property(var_name_1, check_property);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Subsequent set operations should fail */
	status = set_variable(
		var_name_1,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_WRITE_PROTECTED, status);
}

TEST(UefiVariableStoreTests, noRemoveCheck)
{
	efi_status_t status = EFI_SUCCESS;
	std::wstring var_name_1 = L"test_variable_1";
	std::string input_data = "blah blah";

	/* Add a variable */
	status = set_variable(
		var_name_1,
		input_data,
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Apply a check to constrain size to > 0.  This should prevent removal */
	VAR_CHECK_VARIABLE_PROPERTY check_property;
	check_property.Revision = VAR_CHECK_VARIABLE_PROPERTY_REVISION;
	check_property.Attributes = 0;
	check_property.Property = 0;
	check_property.MinSize = 1;
	check_property.MaxSize = 10;

	status = set_check_var_property(var_name_1, check_property);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Try and remove by setting with zero length data */
	status = set_variable(
		var_name_1,
		std::string(),
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_INVALID_PARAMETER, status);

	/* Setting with non zero data should work */
	status = set_variable(
		var_name_1,
		std::string("Good"),
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* But with data that exceeds the MaxSize */
	status = set_variable(
		var_name_1,
		std::string("A data value that exceeds the MaxSize"),
		EFI_VARIABLE_NON_VOLATILE);
	UNSIGNED_LONGS_EQUAL(EFI_INVALID_PARAMETER, status);
}

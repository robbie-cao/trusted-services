/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstring>
#include <service/smm_variable/client/cpp/smm_variable_client.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level tests for the smm-variable service.
 */
TEST_GROUP(SmmVariableServiceTests)
{
	void setup()
	{
		struct rpc_caller *caller;
		int status;

		m_rpc_session_handle = NULL;
		m_service_context = NULL;

		service_locator_init();

		m_service_context =
			service_locator_query("sn:trustedfirmware.org:smm-variable:0", &status);
		CHECK_TRUE(m_service_context);

		m_rpc_session_handle =
			service_context_open(m_service_context, TS_RPC_ENCODING_PACKED_C, &caller);
		CHECK_TRUE(m_rpc_session_handle);

		m_client = new smm_variable_client(caller);

		setup_common_guid();
	}

	void teardown()
	{
		delete m_client;
		m_client = NULL;

		if (m_service_context) {
			if (m_rpc_session_handle) {
				service_context_close(m_service_context, m_rpc_session_handle);
				m_rpc_session_handle = NULL;
			}

			service_context_relinquish(m_service_context);
			m_service_context = NULL;
		}
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

	/* This test makes the irreversible transition from boot to runtime
	 * state, leaving a variable that can't be removed. To prevent this from
	 * breaking the variable enumeration test, this test is called from
	 * the enumeration test to guarantee the order.
	 */
	void runtimeStateAccessControl()
	{
		efi_status_t efi_status = EFI_SUCCESS;
		std::wstring boot_var_name = L"a boot variable";
		std::string boot_set_data = "Only accessible during boot";
		std::wstring runtime_var_name = L"a runtime variable";
		std::string runtime_set_data = "Only accessible during runtime";
		std::string get_data;

		/* This test can only successfully be run once as it exits
		 * boot service, blocking access to the added boot variable.
		 * If the boot variable already exists at the start of the
		 * test, indicating a subsequent test run, just return.
		 */
		efi_status = m_client->get_variable(
			m_common_guid,
			boot_var_name,
			get_data);
		if (efi_status == EFI_ACCESS_DENIED) return;

		/* Add variables with runtime state access control */
		efi_status = m_client->set_variable(
			m_common_guid,
			boot_var_name,
			boot_set_data,
			EFI_VARIABLE_BOOTSERVICE_ACCESS);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		efi_status = m_client->set_variable(
			m_common_guid,
			runtime_var_name,
			runtime_set_data,
			EFI_VARIABLE_NON_VOLATILE |
			EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		/* Expect access to boot variable to be permitted */
		efi_status = m_client->get_variable(
			m_common_guid,
			boot_var_name,
			get_data);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
		UNSIGNED_LONGS_EQUAL(boot_set_data.size(), get_data.size());
		LONGS_EQUAL(0, get_data.compare(boot_set_data));

		/* Expect access to the runtime variable to also be permitted during boot */
		efi_status = m_client->get_variable(
			m_common_guid,
			runtime_var_name,
			get_data);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		/* Exit boot service - access should no longer be permitted */
		efi_status = m_client->exit_boot_service();
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		/* Access to the boot variablel should now be forbidden */
		efi_status = m_client->get_variable(
			m_common_guid,
			boot_var_name,
			get_data);
		UNSIGNED_LONGLONGS_EQUAL(EFI_ACCESS_DENIED, efi_status);

		/* Expect access to the runtime variable should still be permitted */
		efi_status = m_client->get_variable(
			m_common_guid,
			runtime_var_name,
			get_data);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
		UNSIGNED_LONGS_EQUAL(runtime_set_data.size(), get_data.size());
		LONGS_EQUAL(0, get_data.compare(runtime_set_data));

		/* Expect removing boot variable to be forbidden */
		efi_status = m_client->remove_variable(m_common_guid, boot_var_name);
		UNSIGNED_LONGLONGS_EQUAL(EFI_ACCESS_DENIED, efi_status);

		/* Expect removing runtime variable to be permitted */
		efi_status = m_client->remove_variable(m_common_guid, runtime_var_name);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	}

	/* This test also leaves an unremovable variable */
	void setReadOnlyConstraint()
	{
		efi_status_t efi_status = EFI_SUCCESS;
		std::wstring var_name_1 = L"ro_variable";
		std::string set_data = "A read only variable";

		/* Add a variable to the store */
		efi_status = m_client->set_variable(
			m_common_guid,
			var_name_1,
			set_data,
			0);

		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		/* Apply a check to constrain to Read Only */
		VAR_CHECK_VARIABLE_PROPERTY check_property;
		check_property.Revision = VAR_CHECK_VARIABLE_PROPERTY_REVISION;
		check_property.Attributes = 0;
		check_property.Property = VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
		check_property.MinSize = 0;
		check_property.MaxSize = 100;

		efi_status = m_client->set_var_check_property(
			m_common_guid,
			var_name_1,
			check_property);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		/* Read back the check property constraint and expect it to match the set value */
		VAR_CHECK_VARIABLE_PROPERTY got_check_property;

		efi_status = m_client->get_var_check_property(
			m_common_guid,
			var_name_1,
			got_check_property);
		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		UNSIGNED_LONGS_EQUAL(check_property.Revision, got_check_property.Revision);
		UNSIGNED_LONGS_EQUAL(check_property.Attributes, got_check_property.Attributes);
		UNSIGNED_LONGS_EQUAL(check_property.Property, got_check_property.Property);
		UNSIGNED_LONGS_EQUAL(check_property.MinSize, got_check_property.MinSize);
		UNSIGNED_LONGS_EQUAL(check_property.MaxSize, got_check_property.MaxSize);

		/* Attempt to modify variable */
		efi_status = m_client->set_variable(
			m_common_guid,
			var_name_1,
			std::string("Different variable data"),
			0);

		UNSIGNED_LONGLONGS_EQUAL(EFI_WRITE_PROTECTED, efi_status);

		/* Expect to still be able to read variable */
		std::string get_data;

		efi_status = m_client->get_variable(
			m_common_guid,
			var_name_1,
			get_data);

		UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

		/* Variable value should be unmodified */
		UNSIGNED_LONGS_EQUAL(set_data.size(), get_data.size());
		LONGS_EQUAL(0, get_data.compare(set_data));
	}

	smm_variable_client *m_client;
	rpc_session_handle m_rpc_session_handle;
	struct service_context *m_service_context;
	EFI_GUID m_common_guid;
};

TEST(SmmVariableServiceTests, setAndGet)
{
	efi_status_t efi_status = EFI_SUCCESS;
	std::wstring var_name = L"test_variable";
	std::string set_data = "UEFI variable data string";
	std::string get_data;

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name,
		set_data,
		0);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->get_variable(
		m_common_guid,
		var_name,
		get_data);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	UNSIGNED_LONGS_EQUAL(set_data.size(), get_data.size());
	LONGS_EQUAL(0, get_data.compare(set_data));

	/* Extend the variable using an append write */
	std::string append_data = " values added with append write";

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name,
		append_data,
		EFI_VARIABLE_APPEND_WRITE);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->get_variable(
		m_common_guid,
		var_name,
		get_data);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	std::string appended_data = set_data + append_data;

	/* Expect the append write operation to have extended the variable */
	UNSIGNED_LONGLONGS_EQUAL(appended_data.size(), get_data.size());
	LONGS_EQUAL(0, appended_data.compare(get_data));

	/* Expect remove to be permitted */
	efi_status = m_client->remove_variable(m_common_guid, var_name);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
}

TEST(SmmVariableServiceTests, setAndGetNv)
{
	efi_status_t efi_status = EFI_SUCCESS;
	std::wstring var_name = L"an NV test_variable";
	std::string set_data = "Another UEFI variable data string";
	std::string get_data;

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name,
		set_data,
		EFI_VARIABLE_NON_VOLATILE);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->get_variable(
		m_common_guid,
		var_name,
		get_data);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	UNSIGNED_LONGS_EQUAL(set_data.size(), get_data.size());
	LONGS_EQUAL(0, get_data.compare(set_data));

	/* Extend the variable using an append write */
	std::string append_data = " values added with append write";

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name,
		append_data,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_APPEND_WRITE);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->get_variable(
		m_common_guid,
		var_name,
		get_data);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	std::string appended_data = set_data + append_data;

	/* Expect the append write operation to have extended the variable */
	UNSIGNED_LONGLONGS_EQUAL(appended_data.size(), get_data.size());
	LONGS_EQUAL(0, appended_data.compare(get_data));

	/* Expect remove to be permitted */
	efi_status = m_client->remove_variable(m_common_guid, var_name);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
}

TEST(SmmVariableServiceTests, enumerateStoreContents)
{
	efi_status_t efi_status = EFI_SUCCESS;

	/* Query information about the empty variable store */
	size_t nv_max_variable_storage_size = 0;
	size_t nv_max_variable_size = 0;
	size_t nv_remaining_variable_storage_size = 0;

	efi_status = m_client->query_variable_info(
		EFI_VARIABLE_NON_VOLATILE,
		&nv_max_variable_storage_size,
		&nv_remaining_variable_storage_size,
		&nv_max_variable_size);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGLONGS_EQUAL(nv_max_variable_storage_size, nv_remaining_variable_storage_size);

	size_t v_max_variable_storage_size = 0;
	size_t v_max_variable_size = 0;
	size_t v_remaining_variable_storage_size = 0;

	efi_status = m_client->query_variable_info(
		0,
		&v_max_variable_storage_size,
		&v_remaining_variable_storage_size,
		&v_max_variable_size);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGLONGS_EQUAL(v_max_variable_storage_size, v_remaining_variable_storage_size);

	/* Add some variables to the store */
	std::wstring var_name_1 = L"varibale_1";
	std::wstring var_name_2 = L"varibale_2";
	std::wstring var_name_3 = L"varibale_3";
	std::string set_data = "Some variable data";

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_1,
		set_data,
		EFI_VARIABLE_NON_VOLATILE);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_2,
		set_data,
		EFI_VARIABLE_NON_VOLATILE);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_3,
		set_data,
		0);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Query variable info again and check it's as expected */
	size_t max_variable_storage_size = 0;
	size_t max_variable_size = 0;
	size_t remaining_variable_storage_size = 0;

	/* Check non-volatile - two variables have been added */
	efi_status = m_client->query_variable_info(
		EFI_VARIABLE_NON_VOLATILE,
		&max_variable_storage_size,
		&remaining_variable_storage_size,
		&max_variable_size);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGLONGS_EQUAL(
		(nv_remaining_variable_storage_size - set_data.size() * 2),
		remaining_variable_storage_size);

	/* Check volatile - one variables have been added */
	efi_status = m_client->query_variable_info(
		0,
		&max_variable_storage_size,
		&remaining_variable_storage_size,
		&max_variable_size);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGLONGS_EQUAL(
		(v_remaining_variable_storage_size - set_data.size() * 1),
		remaining_variable_storage_size);

	/* Enumerate store contents - expect the values we added */
	std::wstring var_name;
	EFI_GUID guid;
	memset(&guid, 0, sizeof(guid));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(var_name_1.size(), var_name.size());
	LONGS_EQUAL(0, var_name.compare(var_name_1));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(var_name_2.size(), var_name.size());
	LONGS_EQUAL(0, var_name.compare(var_name_2));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(var_name_3.size(), var_name.size());
	LONGS_EQUAL(0, var_name.compare(var_name_3));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGLONGS_EQUAL(EFI_NOT_FOUND, efi_status);

	/* Expect to be able to remove all variables */
	efi_status = m_client->remove_variable(m_common_guid, var_name_1);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->remove_variable(m_common_guid, var_name_2);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->remove_variable(m_common_guid, var_name_3);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Now that the enumeration test is completed, it's safe to
	 * run tests that leavev variables behind.
	 */
	runtimeStateAccessControl();
	setReadOnlyConstraint();
}

TEST(SmmVariableServiceTests, setSizeConstraint)
{
	efi_status_t efi_status = EFI_SUCCESS;
	std::wstring var_name_1 = L"size_limited_variable";
	std::string set_data = "Initial value";

	/* Add a variable to the store */
	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_1,
		set_data,
		0);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Apply a check to constrain the variable size */
	VAR_CHECK_VARIABLE_PROPERTY check_property;
	check_property.Revision = VAR_CHECK_VARIABLE_PROPERTY_REVISION;
	check_property.Attributes = 0;
	check_property.Property = 0;
	check_property.MinSize = 0;
	check_property.MaxSize = 20;

	efi_status = m_client->set_var_check_property(
		m_common_guid,
		var_name_1,
		check_property);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Attempt to set value to a size that exceeds the MaxSize constraint */
	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_1,
		std::string("A data value that exceeds the MaxSize constraint"),
		0);
	UNSIGNED_LONGLONGS_EQUAL(EFI_INVALID_PARAMETER, efi_status);

	/* But setting a value that's within the constraints should work */
	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_1,
		std::string("Small value"),
		0);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Removing should be allowed though */
	efi_status = m_client->remove_variable(m_common_guid, var_name_1);
	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Although the variable has been removed, the constraint should
	 * still be set.
	 */
	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_1,
		std::string("Another try to set a value that exceeds the MaxSize constraint"),
		0);
	UNSIGNED_LONGLONGS_EQUAL(EFI_INVALID_PARAMETER, efi_status);
}

TEST(SmmVariableServiceTests, checkMaxVariablePayload)
{
	efi_status_t efi_status = EFI_SUCCESS;
	size_t max_payload_size = 0;

	/* Expect to read a reasonable size for the variable payload */
	efi_status = m_client->get_payload_size(max_payload_size);

	UNSIGNED_LONGLONGS_EQUAL(EFI_SUCCESS, efi_status);
	CHECK_TRUE(max_payload_size >= 1024);
	CHECK_TRUE(max_payload_size <= 64 * 1024);
}

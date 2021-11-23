/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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

		service_context_close(m_service_context, m_rpc_session_handle);
		m_rpc_session_handle = NULL;

		service_context_relinquish(m_service_context);
		m_service_context = NULL;
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

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->get_variable(
		m_common_guid,
		var_name,
		get_data);

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	UNSIGNED_LONGS_EQUAL(set_data.size(), get_data.size());
	LONGS_EQUAL(0, get_data.compare(set_data));
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

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->get_variable(
		m_common_guid,
		var_name,
		get_data);

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	UNSIGNED_LONGS_EQUAL(set_data.size(), get_data.size());
	LONGS_EQUAL(0, get_data.compare(set_data));
}

TEST(SmmVariableServiceTests, runtimeStateAccessControl)
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
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->set_variable(
		m_common_guid,
		runtime_var_name,
		runtime_set_data,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Expect access to boot variable to be permitted */
	efi_status = m_client->get_variable(
		m_common_guid,
		boot_var_name,
		get_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(boot_set_data.size(), get_data.size());
	LONGS_EQUAL(0, get_data.compare(boot_set_data));

	/* Expect access to the runtime variable to be forbidden during boot */
	efi_status = m_client->get_variable(
		m_common_guid,
		runtime_var_name,
		get_data);
	UNSIGNED_LONGS_EQUAL(EFI_ACCESS_DENIED, efi_status);

	/* Exit boot service - access should no longer be permitted */
	efi_status = m_client->exit_boot_service();
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Access to the boot variablel should now be forbidden */
	efi_status = m_client->get_variable(
		m_common_guid,
		boot_var_name,
		get_data);
	UNSIGNED_LONGS_EQUAL(EFI_ACCESS_DENIED, efi_status);

	/* Expect access to the runtime variable to now be permitted */
	efi_status = m_client->get_variable(
		m_common_guid,
		runtime_var_name,
		get_data);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(runtime_set_data.size(), get_data.size());
	LONGS_EQUAL(0, get_data.compare(set_data));
}

TEST(SmmVariableServiceTests, enumerateStoreContents)
{
	efi_status_t efi_status = EFI_SUCCESS;
	std::wstring var_name_1 = L"varibale_1";
	std::wstring var_name_2 = L"varibale_2";
	std::wstring var_name_3 = L"varibale_3";
	std::string set_data = "Some variable data";

	/* Add some variables to the store */
	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_1,
		set_data,
		EFI_VARIABLE_NON_VOLATILE);

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_2,
		set_data,
		EFI_VARIABLE_NON_VOLATILE);

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	efi_status = m_client->set_variable(
		m_common_guid,
		var_name_3,
		set_data,
		0);

	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);

	/* Enumerate store contents - expect the values we added */
	std::wstring var_name;
	EFI_GUID guid = {0};

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(var_name_1.size(), var_name.size());
	LONGS_EQUAL(0, var_name.compare(var_name_1));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(var_name_2.size(), var_name.size());
	LONGS_EQUAL(0, var_name.compare(var_name_2));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, efi_status);
	UNSIGNED_LONGS_EQUAL(var_name_3.size(), var_name.size());
	LONGS_EQUAL(0, var_name.compare(var_name_3));

	efi_status = m_client->get_next_variable_name(guid, var_name);
	UNSIGNED_LONGS_EQUAL(EFI_NOT_FOUND, efi_status);
}

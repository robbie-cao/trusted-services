/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <wchar.h>
#include <string>
#include <vector>
#include <CppUTest/TestHarness.h>
#include <service/smm_variable/backend/variable_index.h>
#include <service/smm_variable/backend/variable_index_iterator.h>


TEST_GROUP(UefiVariableIndexTests)
{
	void setup()
	{
		efi_status_t status = variable_index_init(&m_variable_index, MAX_VARIABLES);
		UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

		guid_1.Data1 = 0x12341234;
		guid_1.Data2 = 0x1234;
		guid_1.Data3 = 0x1234;
		guid_1.Data4[0] = 0x00;
		guid_1.Data4[1] = 0x01;
		guid_1.Data4[2] = 0x02;
		guid_1.Data4[3] = 0x03;
		guid_1.Data4[4] = 0x04;
		guid_1.Data4[5] = 0x05;
		guid_1.Data4[6] = 0x06;
		guid_1.Data4[7] = 0x07;

		guid_2.Data1 = 0x55443322;
		guid_2.Data2 = 0x2345;
		guid_2.Data3 = 0x2345;
		guid_2.Data4[0] = 0x10;
		guid_2.Data4[1] = 0x11;
		guid_2.Data4[2] = 0x12;
		guid_2.Data4[3] = 0x13;
		guid_2.Data4[4] = 0x14;
		guid_2.Data4[5] = 0x15;
		guid_2.Data4[6] = 0x16;
		guid_2.Data4[7] = 0x17;

		name_1 = to_variable_name(L"var1");
		name_2 = to_variable_name(L"var2_nv");
		name_3 = to_variable_name(L"var3_nv");
	}

	void teardown()
	{
		variable_index_deinit(&m_variable_index);
	}

	std::vector<int16_t> to_variable_name(const std::wstring &string)
	{
		std::vector<int16_t> var_name;

		for (size_t i = 0; i < string.size(); i++) {

			var_name.push_back((int16_t)string[i]);
		}

		var_name.push_back(0);

		return var_name;
	}

	void create_variables()
	{
		const struct variable_info *info = NULL;

		info = variable_index_add_variable(
			&m_variable_index,
			&guid_1,
			name_1.size() * sizeof(int16_t),
			name_1.data(),
			EFI_VARIABLE_BOOTSERVICE_ACCESS);

		CHECK_TRUE(info);

		info = variable_index_add_variable(
			&m_variable_index,
			&guid_2,
			name_2.size() * sizeof(int16_t),
			name_2.data(),
			EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS);

		CHECK_TRUE(info);

		info = variable_index_add_variable(
			&m_variable_index,
			&guid_1,
			name_3.size() * sizeof(int16_t),
			name_3.data(),
			EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS);

		CHECK_TRUE(info);
	}

	static const size_t MAX_VARIABLES = 10;

	struct variable_index m_variable_index;
	EFI_GUID guid_1;
	EFI_GUID guid_2;
	std::vector<int16_t> name_1;
	std::vector<int16_t> name_2;
	std::vector<int16_t> name_3;
};

TEST(UefiVariableIndexTests, emptyIndexOperations)
{
	const struct variable_info *info = NULL;

	/* Expect not to find a variable */
	info = variable_index_find(
		&m_variable_index,
		&guid_1,
		name_1.size() * sizeof(int16_t),
		name_1.data());
	POINTERS_EQUAL(NULL, info);

	/* Expect also not to find the next variable */
	info = variable_index_find_next(
		&m_variable_index,
		&guid_1,
		name_1.size() * sizeof(int16_t),
		name_1.data());
	POINTERS_EQUAL(NULL, info);

	/* Remove should silently return */
	variable_index_remove_variable(
		&m_variable_index,
		info);
}

TEST(UefiVariableIndexTests, addWithOversizedName)
{
	const struct variable_info *info = NULL;
	std::vector<int16_t> name;

	name = to_variable_name(L"a long variable name that exceeds the length limit");

	info = variable_index_add_variable(
		&m_variable_index,
		&guid_1,
		name.size() * sizeof(int16_t),
		name.data(),
		EFI_VARIABLE_BOOTSERVICE_ACCESS);

	/* Expect the add to fail because of an oversized name */
	POINTERS_EQUAL(NULL, info);

	name = to_variable_name(L"a long variable name that fits!");

	info = variable_index_add_variable(
		&m_variable_index,
		&guid_1,
		name.size() * sizeof(int16_t),
		name.data(),
		EFI_VARIABLE_BOOTSERVICE_ACCESS);

	/* Expect the add succeed */
	CHECK_TRUE(info);
}

TEST(UefiVariableIndexTests, variableIndexFull)
{
	const struct variable_info *info = NULL;
	EFI_GUID guid = guid_1;

	/* Expect to be able to fill the index */
	for (size_t i = 0; i < MAX_VARIABLES; ++i) {

		info = variable_index_add_variable(
			&m_variable_index,
			&guid,
			name_1.size() * sizeof(int16_t),
			name_1.data(),
			EFI_VARIABLE_BOOTSERVICE_ACCESS);

		CHECK_TRUE(info);

		/* Modify the guid for the next add */
		guid.Data1 += 1;
	}

	/* Variable index should now be full */
	info = variable_index_add_variable(
		&m_variable_index,
		&guid,
		name_1.size() * sizeof(int16_t),
		name_1.data(),
		EFI_VARIABLE_BOOTSERVICE_ACCESS);

	POINTERS_EQUAL(NULL, info);
}


TEST(UefiVariableIndexTests, enumerateStore)
{
	const struct variable_info *info = NULL;
	const std::vector<int16_t> null_name = to_variable_name(L"");

	create_variables();

	info = variable_index_find_next(
		&m_variable_index,
		&guid_1,
		null_name.size() * sizeof(int16_t),
		null_name.data());
	CHECK_TRUE(info);
	LONGS_EQUAL(EFI_VARIABLE_BOOTSERVICE_ACCESS, info->metadata.attributes);
	MEMCMP_EQUAL(&guid_1, &info->metadata.guid, sizeof(EFI_GUID));
	MEMCMP_EQUAL(name_1.data(), info->metadata.name, name_1.size());

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	CHECK_TRUE(info);
	LONGS_EQUAL(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS, info->metadata.attributes);
	MEMCMP_EQUAL(&guid_2, &info->metadata.guid, sizeof(EFI_GUID));
	MEMCMP_EQUAL(name_2.data(), info->metadata.name, name_2.size());

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	CHECK_TRUE(info);
	LONGS_EQUAL(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS, info->metadata.attributes);
	MEMCMP_EQUAL(&guid_1, &info->metadata.guid, sizeof(EFI_GUID));
	MEMCMP_EQUAL(name_3.data(), info->metadata.name, name_3.size());

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	POINTERS_EQUAL(NULL, info);
}

TEST(UefiVariableIndexTests, dumpLoadRoadtrip)
{
	uint8_t buffer[MAX_VARIABLES * sizeof(struct variable_metadata)];

	create_variables();

	/* Expect the info for two NV variables to have been dumped */
	size_t dump_len = 0;
	bool is_dirty = variable_index_dump(&m_variable_index, sizeof(buffer), buffer, &dump_len);

	CHECK_TRUE(is_dirty);
	UNSIGNED_LONGS_EQUAL((sizeof(struct variable_metadata) * 2), dump_len);

	/* Expect no records to be dirty when the dump is repeated */
	dump_len = 0;
	is_dirty = variable_index_dump(&m_variable_index, sizeof(buffer), buffer, &dump_len);

	CHECK_FALSE(is_dirty);
	UNSIGNED_LONGS_EQUAL((sizeof(struct variable_metadata) * 2), dump_len);

	/* Tear down and reinitialize to simulate a reboot */
	variable_index_deinit(&m_variable_index);
	efi_status_t status = variable_index_init(&m_variable_index, MAX_VARIABLES);
	UNSIGNED_LONGS_EQUAL(EFI_SUCCESS, status);

	/* Load the dumped contents */
	size_t load_len = variable_index_restore(&m_variable_index, dump_len, buffer);
	UNSIGNED_LONGS_EQUAL(dump_len, load_len);

	/* Enumerate and now expect only NV variables to be present */
	const struct variable_info *info = NULL;
	std::vector<int16_t> null_name = to_variable_name(L"");

	info = variable_index_find_next(
		&m_variable_index,
		&guid_1,
		null_name.size() * sizeof(int16_t),
		null_name.data());
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
		info->metadata.attributes);

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS,
		info->metadata.attributes);

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	POINTERS_EQUAL(NULL, info);
}

TEST(UefiVariableIndexTests, dumpBufferTooSmall)
{
	uint8_t buffer[1 * sizeof(struct variable_metadata) + 1];

	create_variables();

	/* There should be two NV variables whose info needs saving. The buffer provided
	 * however is only big enough for one. Expect the dumped data length to not
	 * exceed the length of the buffer.
	 */
	size_t dump_len = 0;
	bool is_dirty = variable_index_dump(&m_variable_index, sizeof(buffer), buffer, &dump_len);

	CHECK_TRUE(is_dirty);
	UNSIGNED_LONGS_EQUAL(sizeof(struct variable_metadata) * 1, dump_len);
}


TEST(UefiVariableIndexTests, removeVariable)
{
	uint8_t buffer[MAX_VARIABLES * sizeof(struct variable_metadata)];
	const struct variable_info *info = NULL;

	create_variables();

	/* Remove one of the NV variables */
	info = variable_index_find(
		&m_variable_index,
		&guid_2,
		name_2.size() * sizeof(int16_t),
		name_2.data());

	variable_index_remove_variable(
		&m_variable_index,
		info);

	/* Expect index to be dirty and for only one NV variable to be left */
	size_t dump_len = 0;
	bool is_dirty = variable_index_dump(&m_variable_index, sizeof(buffer), buffer, &dump_len);

	CHECK_TRUE(is_dirty);
	UNSIGNED_LONGS_EQUAL((sizeof(struct variable_metadata) * 1), dump_len);

	/* Remove the volatile variable */
	info = variable_index_find(
		&m_variable_index,
		&guid_1,
		name_1.size() * sizeof(int16_t),
		name_1.data());

	variable_index_remove_variable(
		&m_variable_index,
		info);

	/* Expect index not to be dirty because there was no change to any NV variable */
	dump_len = 0;
	is_dirty = variable_index_dump(&m_variable_index, sizeof(buffer), buffer, &dump_len);

	CHECK_FALSE(is_dirty);
	UNSIGNED_LONGS_EQUAL((sizeof(struct variable_metadata) * 1), dump_len);

	/* Remove the remaining NV variable */
	info = variable_index_find(
		&m_variable_index,
		&guid_1,
		name_3.size() * sizeof(int16_t),
		name_3.data());

	variable_index_remove_variable(
		&m_variable_index,
		info);

	/* Expect index to be dirty and dump to now be empty */
	dump_len = 0;
	is_dirty = variable_index_dump(&m_variable_index, sizeof(buffer), buffer, &dump_len);

	CHECK_TRUE(is_dirty);
	UNSIGNED_LONGS_EQUAL((sizeof(struct variable_metadata) * 0), dump_len);

	/* Enumerate and now expect an empty index */
	info = NULL;
	std::vector<int16_t> null_name = to_variable_name(L"");

	info = variable_index_find_next(
		&m_variable_index,
		&guid_1,
		null_name.size() * sizeof(int16_t),
		null_name.data());
	POINTERS_EQUAL(NULL, info);
}

TEST(UefiVariableIndexTests, checkIterator)
{
	const struct variable_info *info = NULL;

	create_variables();

	struct variable_index_iterator iter;

	variable_index_iterator_first(&iter, &m_variable_index);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check first entry is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_1.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_1.data(), info->metadata.name, info->metadata.name_size);

	variable_index_iterator_next(&iter);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check next is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_2.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_2.data(), info->metadata.name, info->metadata.name_size);

	const struct variable_info *info_to_remove = info;

	variable_index_iterator_next(&iter);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check next is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_3.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_3.data(), info->metadata.name, info->metadata.name_size);

	/* Expect iterating to be done */
	variable_index_iterator_next(&iter);
	CHECK_TRUE(variable_index_iterator_is_done(&iter));

	/* Now remove the middle entry */
	variable_index_remove_variable(&m_variable_index, info_to_remove);

	/* Iterate again but this time there should only be two entries */
	variable_index_iterator_first(&iter, &m_variable_index);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check first entry is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_1.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_1.data(), info->metadata.name, info->metadata.name_size);

	variable_index_iterator_next(&iter);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check next entry is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_3.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_3.data(), info->metadata.name, info->metadata.name_size);

	/* Expect iterating to be done */
	variable_index_iterator_next(&iter);
	CHECK_TRUE(variable_index_iterator_is_done(&iter));
}

TEST(UefiVariableIndexTests, setCheckConstraintsExistingVar)
{
	/* Variable check constraints are set using an independent SMM
	 * function from setting a variable. A client may set constraints
	 * for an existing variable or a non-existing one. This test case
	 * covers setting check constraints for an existing variable.
	 */
	create_variables();

	struct variable_constraints constraints;
	constraints.revision = 10;
	constraints.property = VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
	constraints.attributes = 0;
	constraints.min_size = 1;
	constraints.max_size = 100;

	/* Set check constraints on one of the variables */
	const struct variable_info *info = variable_index_find(
		&m_variable_index,
		&guid_2,
		name_2.size() * sizeof(int16_t),
		name_2.data());

	CHECK_TRUE(info);
	CHECK_TRUE(info->is_variable_set);
	CHECK_FALSE(info->is_constraints_set);

	variable_index_update_constraints(info, &constraints);

	CHECK_TRUE(info->is_constraints_set);
	CHECK_TRUE(info->is_variable_set);

	/* Remove the variable but still expect the variable to be indexed
	 * because of the set constraints.
	 */
	variable_index_remove_variable(
		&m_variable_index,
		info);

	info = variable_index_find(
		&m_variable_index,
		&guid_2,
		name_2.size() * sizeof(int16_t),
		name_2.data());

	CHECK_TRUE(info);
	CHECK_FALSE(info->is_variable_set);
	CHECK_TRUE(info->is_constraints_set);

	/* Enumerate over variables, only expecting to find the two remaining 'set' variables. */
	info = NULL;
	std::vector<int16_t> null_name = to_variable_name(L"");

	info = variable_index_find_next(
		&m_variable_index,
		&guid_1,
		null_name.size() * sizeof(int16_t),
		null_name.data());
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(EFI_VARIABLE_BOOTSERVICE_ACCESS, info->metadata.attributes);

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS,
		info->metadata.attributes);

	info = variable_index_find_next(
		&m_variable_index,
		&info->metadata.guid,
		info->metadata.name_size,
		info->metadata.name);
	CHECK_FALSE(info);

	/* Iterating over the index should still return all three because the set constraints
	 * for variable 2 still persist.
	 */
	struct variable_index_iterator iter;

	variable_index_iterator_first(&iter, &m_variable_index);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check first entry is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_1.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_1.data(), info->metadata.name, info->metadata.name_size);

	variable_index_iterator_next(&iter);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check next is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_2.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_2.data(), info->metadata.name, info->metadata.name_size);

	variable_index_iterator_next(&iter);
	CHECK_FALSE(variable_index_iterator_is_done(&iter));

	/* Check next is as expected */
	info = variable_index_iterator_current(&iter);
	CHECK_TRUE(info);
	UNSIGNED_LONGS_EQUAL(name_3.size() * sizeof(int16_t), info->metadata.name_size);
	MEMCMP_EQUAL(name_3.data(), info->metadata.name, info->metadata.name_size);

	/* Expect iterating to be done */
	variable_index_iterator_next(&iter);
	CHECK_TRUE(variable_index_iterator_is_done(&iter));
}

TEST(UefiVariableIndexTests, setCheckConstraintsNonExistingVar)
{
	/* This test case covers setting check constraints for a variable
	 * that hasn't been set yet.
	 */
	struct variable_constraints constraints;
	constraints.revision = 10;
	constraints.property = VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY;
	constraints.attributes = 0;
	constraints.min_size = 1;
	constraints.max_size = 100;

	/* Initially expect no variable_info */
	const struct variable_info *info = variable_index_find(
		&m_variable_index,
		&guid_2,
		name_2.size() * sizeof(int16_t),
		name_2.data());

	CHECK_FALSE(info);

	/* Adding the check constraints should result in an entry being added */
	info = variable_index_add_constraints(
		&m_variable_index,
		&guid_2,
		name_2.size() * sizeof(int16_t),
		name_2.data(),
		&constraints);

	CHECK_TRUE(info);
	CHECK_FALSE(info->is_variable_set);
	CHECK_TRUE(info->is_constraints_set);

	/* Updating the variable should cause the variable to be marked as set */
	variable_index_update_variable(info, EFI_VARIABLE_RUNTIME_ACCESS);

	CHECK_TRUE(info->is_variable_set);
	CHECK_TRUE(info->is_constraints_set);

	/* Check the constraints are as expected */
	UNSIGNED_LONGS_EQUAL(constraints.revision, info->check_constraints.revision);
	UNSIGNED_LONGS_EQUAL(constraints.property, info->check_constraints.property);
	UNSIGNED_LONGS_EQUAL(constraints.attributes, info->check_constraints.attributes);
	UNSIGNED_LONGS_EQUAL(constraints.min_size, info->check_constraints.min_size);
	UNSIGNED_LONGS_EQUAL(constraints.max_size, info->check_constraints.max_size);
}

#
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UnitTest)

unit_test_add_suite(
	NAME libsp_mock_assert
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/test/mock_assert.cpp
		${CMAKE_CURRENT_LIST_DIR}/test/test_mock_assert.cpp
	INCLUDE_DIRECTORIES
		${CMAKE_CURRENT_LIST_DIR}/include/
		${PROJECT_PATH}/components/common/utils/include
	COMPILE_DEFINITIONS
		-DARM64
)

unit_test_add_suite(
	NAME libsp_mock_ffa_internal_api
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/test/mock_ffa_internal_api.cpp
		${CMAKE_CURRENT_LIST_DIR}/test/test_mock_ffa_internal_api.cpp
	INCLUDE_DIRECTORIES
		${CMAKE_CURRENT_LIST_DIR}/include/
		${PROJECT_PATH}/components/common/utils/include
	COMPILE_DEFINITIONS
		-DARM64
)


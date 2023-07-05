/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SERVICE_UNDER_TEST_H
#define SERVICE_UNDER_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Locate and open an RPC session for the service under test.  Concrete
 * implementations of this function will locate a specific service and
 * associate an RPC Caller with the singleton PSA API client used by
 * the API tests.
 */
int locate_service_under_test(void);

/**
 * Reliquish the RPC session when the test run is complete.
 */
int relinquish_service_under_test(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SERVICE_UNDER_TEST_H */

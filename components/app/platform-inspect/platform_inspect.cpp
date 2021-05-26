/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <psa/crypto.h>
#include <service_locator.h>
#include <service/attestation/reporter/dump/raw/raw_report_dump.h>
#include <service/attestation/reporter/dump/pretty/pretty_report_dump.h>
#include "attest_report_fetcher.h"

int main(int argc, char *argv[])
{
    int rval = -1;

    psa_crypto_init();
    service_locator_init();

    /* Fetch platform info */
    std::string error_msg;
    std::vector<uint8_t> attest_report;

    if (fetch_attest_report(attest_report, error_msg)) {

        rval = pretty_report_dump(attest_report.data(), attest_report.size());
    }
    else {

        printf("%s\n", error_msg.c_str());
    }

    return rval;
}

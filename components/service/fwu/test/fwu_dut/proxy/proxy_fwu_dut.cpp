/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cassert>
#include <service/fwu/test/fwu_client/remote/remote_fwu_client.h>
#include <service/fwu/test/metadata_fetcher/client/client_metadata_fetcher.h>
#include <service/fwu/test/metadata_checker/metadata_checker_v1.h>
#include "proxy_fwu_dut.h"

proxy_fwu_dut::proxy_fwu_dut(
	unsigned int num_locations,
	fwu_dut *remote_dut) :
	fwu_dut(),
	m_num_locations(num_locations),
	m_remote_dut(remote_dut)
{

}

proxy_fwu_dut::~proxy_fwu_dut()
{
	delete m_remote_dut;
	m_remote_dut = NULL;
}

void proxy_fwu_dut::boot(bool from_active_bank)
{
	assert(m_remote_dut);
	m_remote_dut->boot(from_active_bank);
}

void proxy_fwu_dut::shutdown(void)
{
	assert(m_remote_dut);
	m_remote_dut->shutdown();
}

struct boot_info proxy_fwu_dut::get_boot_info(void) const
{
	assert(m_remote_dut);
	return m_remote_dut->get_boot_info();
}

metadata_checker *proxy_fwu_dut::create_metadata_checker(bool is_primary) const
{
	(void)is_primary;

	/* Use service interface to fetch metadata as volume access is no possible */
	fwu_client *fwu_client = new remote_fwu_client;
	metadata_fetcher *metadata_fetcher = new client_metadata_fetcher(fwu_client);

	return new metadata_checker_v1(metadata_fetcher, m_num_locations);
}

fwu_client *proxy_fwu_dut::create_fwu_client(void)
{
	/* Access service via RPC */
	return new remote_fwu_client;
}

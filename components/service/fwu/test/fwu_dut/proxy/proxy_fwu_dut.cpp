/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cassert>
#include "proxy_fwu_dut.h"

proxy_fwu_dut::proxy_fwu_dut(fwu_dut *remote_dut) :
	fwu_dut(),
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
	return NULL;
}

fwu_client *proxy_fwu_dut::create_fwu_client(void)
{
	return NULL;
}

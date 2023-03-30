/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fwu_service_context.h"

struct rpc_interface *fwu_service_context::m_provider_iface = NULL;

fwu_service_context::fwu_service_context(const char *sn)
	: standalone_service_context(sn)
	, m_rpc_demux()
{
}

fwu_service_context::~fwu_service_context()
{
}

void fwu_service_context::set_provider(struct rpc_interface *iface)
{
	m_provider_iface = iface;
}

void fwu_service_context::do_init()
{
	/* For the case where no service interface has been provided,
	 * use an rpc_demux with nothing attached. This will safely
	 * return an error if call requests are made to the rpc endpoint.
	 */
	struct rpc_interface *rpc_iface = rpc_demux_init(&m_rpc_demux);

	if (m_provider_iface) {
		/* A service provider is available. This facility allows a test
		 * case to apply an fwu_provider with a particular configuration
		 * to suite test goals.
		 */
		rpc_iface = m_provider_iface;
	}

	standalone_service_context::set_rpc_interface(rpc_iface);
}

void fwu_service_context::do_deinit()
{
	rpc_demux_deinit(&m_rpc_demux);
	set_provider(NULL);
}

void fwu_service_context_set_provider(struct rpc_interface *iface)
{
	fwu_service_context::set_provider(iface);
}
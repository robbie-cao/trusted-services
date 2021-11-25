/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "standalone_service_context.h"
#include <cassert>

/* Concrete C service_context methods */
static rpc_session_handle standalone_service_context_open(void *context, struct rpc_caller **caller);
static void standalone_service_context_close(void *context, rpc_session_handle session_handle);
static void standalone_service_context_relinquish(void *context);


standalone_service_context::standalone_service_context(const char *sn) :
	m_sn(sn),
	m_ref_count(0),
	m_rpc_buffer_size_override(0),
	m_service_context(),
	m_rpc_interface(NULL)
{
	m_service_context.context = this;
	m_service_context.open = standalone_service_context_open;
	m_service_context.close = standalone_service_context_close;
	m_service_context.relinquish = standalone_service_context_relinquish;
}

standalone_service_context::standalone_service_context(
	const char *sn,
	size_t rpc_buffer_size_override) :
	m_sn(sn),
	m_ref_count(0),
	m_rpc_buffer_size_override(rpc_buffer_size_override),
	m_service_context(),
	m_rpc_interface(NULL)
{
	m_service_context.context = this;
	m_service_context.open = standalone_service_context_open;
	m_service_context.close = standalone_service_context_close;
	m_service_context.relinquish = standalone_service_context_relinquish;
}

standalone_service_context::~standalone_service_context()
{

}

void standalone_service_context::init()
{
	assert(m_ref_count >= 0);

	if (!m_ref_count) do_init();
	++m_ref_count;
}

void standalone_service_context::deinit()
{
	assert(m_ref_count > 0);

	--m_ref_count;
	if (!m_ref_count) do_deinit();
}

rpc_session_handle standalone_service_context::open(struct rpc_caller **caller)
{
	struct rpc_session *session = new rpc_session(m_rpc_interface, m_rpc_buffer_size_override);
	*caller = session->m_rpc_caller;
	return static_cast<rpc_session_handle>(session);
}

void standalone_service_context::close(rpc_session_handle session_handle)
{
	struct rpc_session *session = reinterpret_cast<struct rpc_session*>(session_handle);
	delete session;
}

const std::string &standalone_service_context::get_service_name() const
{
	return m_sn;
}

struct service_context *standalone_service_context::get_service_context()
{
	return &m_service_context;
}

void standalone_service_context::set_rpc_interface(rpc_interface *iface)
{
	m_rpc_interface = iface;
}

standalone_service_context::rpc_session::rpc_session(
	struct rpc_interface *rpc_interface,
	size_t rpc_buffer_size_override) :
	m_direct_caller(),
	m_rpc_caller()
{
	m_rpc_caller = (rpc_buffer_size_override) ?
		direct_caller_init(&m_direct_caller, rpc_interface,
			rpc_buffer_size_override, rpc_buffer_size_override) :
		direct_caller_init_default(&m_direct_caller, rpc_interface);
}

standalone_service_context::rpc_session::~rpc_session()
{
	direct_caller_deinit(&m_direct_caller);
}

static rpc_session_handle standalone_service_context_open(void *context, struct rpc_caller **caller)
{
	rpc_session_handle handle = NULL;
	standalone_service_context *this_context = reinterpret_cast<standalone_service_context*>(context);

	if (this_context) {
		handle = this_context->open(caller);
	}

	return handle;
}

static void standalone_service_context_close(void *context, rpc_session_handle session_handle)
{
	standalone_service_context *this_context = reinterpret_cast<standalone_service_context*>(context);

	if (this_context) {
		this_context->close(session_handle);
	}
}

static void standalone_service_context_relinquish(void *context)
{
	standalone_service_context *this_context = reinterpret_cast<standalone_service_context*>(context);

	if (this_context) {
		this_context->deinit();
	}
}

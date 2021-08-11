/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <service/crypto/client/cpp/protocol/packed-c/packedc_crypto_client.h>
#include <service/crypto/test/service/extension/key_derivation/key_derivation_service_scenarios.h>
#include <protocols/rpc/common/packed-c/encoding.h>
#include <service_locator.h>
#include <CppUTest/TestHarness.h>

/*
 * Service-level key derivation tests that use the packed-c access protocol serialization
 */
TEST_GROUP(CryptoKeyDerivationServicePackedcTests)
{
	void setup()
	{
		struct rpc_caller *caller;
		int status;

		m_rpc_session_handle = NULL;
		m_crypto_service_context = NULL;
		m_scenarios = NULL;

		service_locator_init();

		m_crypto_service_context = service_locator_query("sn:trustedfirmware.org:crypto:0", &status);
		CHECK_TRUE(m_crypto_service_context);

		m_rpc_session_handle = service_context_open(m_crypto_service_context, TS_RPC_ENCODING_PACKED_C, &caller);
		CHECK_TRUE(m_rpc_session_handle);

		m_scenarios = new key_derivation_service_scenarios(new packedc_crypto_client(caller));
	}

	void teardown()
	{
		delete m_scenarios;
		m_scenarios = NULL;

		service_context_close(m_crypto_service_context, m_rpc_session_handle);
		m_rpc_session_handle = NULL;

		service_context_relinquish(m_crypto_service_context);
		m_crypto_service_context = NULL;
	}

	rpc_session_handle m_rpc_session_handle;
	struct service_context *m_crypto_service_context;
	key_derivation_service_scenarios *m_scenarios;
};

TEST(CryptoKeyDerivationServicePackedcTests, hkdfDeriveKey)
{
	m_scenarios->hkdfDeriveKey();
}

TEST(CryptoKeyDerivationServicePackedcTests, hkdfDeriveBytes)
{
	m_scenarios->hkdfDeriveBytes();
}

TEST(CryptoKeyDerivationServicePackedcTests, deriveAbort)
{
	m_scenarios->deriveAbort();
}

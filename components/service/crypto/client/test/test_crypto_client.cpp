/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cassert>
#include "test_crypto_client.h"

test_crypto_client::factory *test_crypto_client::m_default_factory = NULL;

test_crypto_client::test_crypto_client() :
    protobuf_crypto_client(),
    m_is_initialized(false),
    m_injected_faults()
{

}

test_crypto_client::~test_crypto_client()
{
    deinit();
}

bool test_crypto_client::init()
{
    bool success = !m_is_initialized;
    m_is_initialized = true;
    return success;
}

bool test_crypto_client::deinit()
{
    bool success = m_is_initialized;
    m_is_initialized = false;
    return success;
}

test_crypto_client *test_crypto_client::create_default()
{
    /*
     * It's mandatory to include a concrete test_crypto_client
     * that registers its own factory in a deployment if you
     * want to use this factory method.
     */
    assert(m_default_factory);
    return m_default_factory->create();
}

void test_crypto_client::register_factory(factory *factory)
{
    /*
     * Don't allow overriding of an existing default.  This
     * will happen if two test_crypto_client components have
     * been included in a deployment.
     */
    assert(!m_default_factory);
    m_default_factory = factory;
}

void test_crypto_client::deregister_factory(factory *factory)
{
    if (m_default_factory == factory) m_default_factory = NULL;
}

bool test_crypto_client::inject_fault(enum fault_code code)
{
    assert(!m_is_initialized);

    bool is_supported = is_fault_supported(code);
    if (is_supported) m_injected_faults.push_back(code);
    return is_supported;
}

bool test_crypto_client::is_fault_supported(enum fault_code code) const
{
    /* Derived classes may override this if fault simualtion is supported */
    (void)code;
    return false;
}

bool test_crypto_client::is_fault_injected(enum fault_code code) const
{
    bool is_injected = false;

    for (size_t i = 0; !is_injected && i < m_injected_faults.size(); ++i) {
        is_injected = (m_injected_faults[i] == code);
    }

    return is_injected;
}

/*
 * Test methods by default are not supported.  Calling a non-supported
 * method will trigger an assert.  A class derived from this one may
 * pick and choose which test methods it supports.
 */
bool test_crypto_client::keystore_reset_is_supported() const { return false; }
void test_crypto_client::keystore_reset() { assert(false); }

bool test_crypto_client::keystore_key_exists_is_supported() const { return false; }
bool test_crypto_client::keystore_key_exists(uint32_t id) const { (void)id; assert(false); return false; }

bool test_crypto_client::keystore_keys_held_is_supported() const { return false; }
size_t test_crypto_client::keystore_keys_held() const { assert(false); return 0; }
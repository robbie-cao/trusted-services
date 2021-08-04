/*
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_CRYPTO_CLIENT_H
#define TEST_CRYPTO_CLIENT_H

#include <service/crypto/client/cpp/protocol/protobuf/protobuf_crypto_client.h>
#include <vector>

/*
 * A specialization of the crypto_client class that extends it to add
 * virtial methods to support test.  Depending on the deployment,
 * real implementations of test methods may or may not exist.  For example,
 * for a real distributed deployment where the key store is located in
 * a secure processing environment, back door test methods that peak
 * into the keystore are clearly not possible (or at least desirable!).
 * Each virtual test method is paired with a is_supported() method to
 * allow test cases to adapt to circumstances.
 */
class test_crypto_client : public protobuf_crypto_client
{
public:
    virtual ~test_crypto_client();

    virtual bool init();
    virtual bool deinit();

    /*
     * A factory method for contsructing the default class
     * of test_crypto_client for the deployment.
     */
    static test_crypto_client *create_default();

    /*
     * Fault conditions that may be injected to allow error
     * handling to be tested.
     */
    enum fault_code
    {
        FAILED_TO_DISCOVER_SECURE_STORAGE
    };

    /*
     * Injects the specified fault.  May be called multiple
     * times to inject different fault conditions.  Faults
     * should be injected prior to calling the init() method
     * to allow startup faults to be simulated.  Returns true
     * if the fault condition can be simulated.
     */
    bool inject_fault(enum fault_code code);

    /* Wipe all keys held in the keystore */
    virtual bool keystore_reset_is_supported() const;
    virtual void keystore_reset();

    /* Check if a key is held in the keystore */
    virtual bool keystore_key_exists_is_supported() const;
    virtual bool keystore_key_exists(uint32_t id) const;

    /* Return the number of keys in the keystore */
    virtual bool keystore_keys_held_is_supported() const;
    virtual size_t keystore_keys_held() const;

    /* An abstract factory for constructing concrete test_crypto_client objects */
    class factory
    {
    public:
        virtual test_crypto_client *create() = 0;
    };

    static void register_factory(factory *factory);
    static void deregister_factory(factory *factory);

protected:
    test_crypto_client();
    virtual bool is_fault_supported(enum fault_code code) const;
    bool is_fault_injected(enum fault_code code) const;

private:
    bool m_is_initialized;
    std::vector<fault_code> m_injected_faults;
    static factory *m_default_factory;
};

#endif /* STANDALONE_CRYPTO_CLIENT_H */

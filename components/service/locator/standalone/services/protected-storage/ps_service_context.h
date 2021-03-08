/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDALONE_PS_SERVICE_CONTEXT_H
#define STANDALONE_PS_SERVICE_CONTEXT_H

#include <service/locator/standalone/standalone_service_context.h>
#include <service/secure_storage/frontend/secure_storage_provider/secure_storage_provider.h>
#include <service/secure_storage/backend/mock_store/mock_store.h>

class ps_service_context : public standalone_service_context
{
public:
    ps_service_context(const char *sn);
    virtual ~ps_service_context();

private:

    void do_init();
    void do_deinit();

    struct secure_storage_provider m_storage_provider;
    struct mock_store m_mock_store;
};

#endif /* STANDALONE_PS_SERVICE_CONTEXT_H */

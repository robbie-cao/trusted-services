/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <stdbool.h>
#include <protocols/service/test_runner/packed-c/opcodes.h>
#include <protocols/service/test_runner/packed-c/status.h>
#include <protocols/rpc/common/packed-c/status.h>
#include "test_runner_provider.h"
#include "test_runner_backend.h"

/* Service request handlers */
static rpc_status_t run_tests_handler(void *context, struct call_req* req);
static rpc_status_t list_tests_handler(void *context, struct call_req* req);

/* Handler mapping table for service */
static const struct service_handler handler_table[] = {
    {TS_TEST_RUNNER_OPCODE_RUN_TESTS,          run_tests_handler},
    {TS_TEST_RUNNER_OPCODE_LIST_TESTS,         list_tests_handler}
};

struct rpc_interface *test_runner_provider_init(struct test_runner_provider *context)
{
    struct rpc_interface *rpc_interface = NULL;

    if (context) {

        for (size_t encoding = 0; encoding < TS_RPC_ENCODING_LIMIT; ++encoding)
            context->serializers[encoding] = NULL;

        context->backend_list = NULL;

        service_provider_init(&context->base_provider, context,
                    handler_table, sizeof(handler_table)/sizeof(struct service_handler));

        rpc_interface = service_provider_get_rpc_interface(&context->base_provider);

        /* Allow a deployment specific test_runner backend to be registrered */
        test_runner_register_default_backend(context);
    }

    return rpc_interface;
}

void test_runner_provider_deinit(struct test_runner_provider *context)
{
    (void)context;
}

void test_runner_provider_register_serializer(struct test_runner_provider *context,
                unsigned int encoding, const struct test_runner_provider_serializer *serializer)
{
    if (encoding < TS_RPC_ENCODING_LIMIT)
        context->serializers[encoding] = serializer;
}

void test_runner_provider_register_backend(struct test_runner_provider *context,
                struct test_runner_backend *backend)
{
    /* Insert into list of backend test runners */
    backend->next = context->backend_list;
    context->backend_list = backend;
}

static const struct test_runner_provider_serializer* get_test_runner_serializer(
                struct test_runner_provider *context, const struct call_req *req)
{
    const struct test_runner_provider_serializer* serializer = NULL;
    unsigned int encoding = call_req_get_encoding(req);

    if (encoding < TS_RPC_ENCODING_LIMIT) serializer = context->serializers[encoding];

    return serializer;
}

static struct test_result *alloc_result_buf(struct test_runner_provider *context,
                const struct test_spec *test_spec, size_t *result_limit)
{
    struct test_result *space = NULL;
    size_t total_tests = 0;
    struct test_runner_backend *backend = context->backend_list;

    while (backend) {

        total_tests += backend->count_tests(test_spec);
        backend = backend->next;
    }

    space = malloc(total_tests * sizeof(struct test_result));

    *result_limit = total_tests;
    return space;
}

static int run_qualifying_tests(struct test_runner_provider *context, bool list_only, const struct test_spec *spec,
		        struct test_summary *summary, struct test_result *results, size_t result_limit)
{
    int test_status = TS_TEST_RUNNER_STATUS_SUCCESS;
    size_t results_used = 0;
    struct test_runner_backend *backend = context->backend_list;

    summary->num_tests = 0;
	summary->num_results = 0;
	summary->num_passed = 0;
	summary->num_failed = 0;

    while (backend && (test_status == TS_TEST_RUNNER_STATUS_SUCCESS)) {

        struct test_summary interim_summary;

        if (list_only) {

            backend->list_tests(spec, &interim_summary,
                            &results[summary->num_results],
                            result_limit - summary->num_results);
        }
        else {

            test_status = backend->run_tests(spec, &interim_summary,
                            &results[summary->num_results],
                            result_limit - summary->num_results);
        }

        summary->num_tests += interim_summary.num_tests;
	    summary->num_results += interim_summary.num_results;
	    summary->num_passed += interim_summary.num_passed;
	    summary->num_failed += interim_summary.num_failed;

        backend = backend->next;
    }

    return test_status;
}

static rpc_status_t run_tests_handler(void *context, struct call_req* req)
{
    struct test_runner_provider *this_instance = (struct test_runner_provider*)context;
    rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
    struct test_spec test_spec;

    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct test_runner_provider_serializer *serializer = get_test_runner_serializer(this_instance, req);

    if (serializer)
        rpc_status = serializer->deserialize_run_tests_req(req_buf, &test_spec);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        int test_status;
        struct test_summary summary;
        size_t result_limit = 0;
        struct test_result *result_buf = alloc_result_buf(this_instance, &test_spec, &result_limit);

        test_status = run_qualifying_tests(this_instance, false, &test_spec, &summary, result_buf, result_limit);

        call_req_set_opstatus(req, test_status);

        if (test_status == TS_TEST_RUNNER_STATUS_SUCCESS) {

            struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
            rpc_status = serializer->serialize_run_tests_resp(resp_buf, &summary, result_buf);

            free(result_buf);
        }
    }

    return rpc_status;
}

static rpc_status_t list_tests_handler(void *context, struct call_req* req)
{
    struct test_runner_provider *this_instance = (struct test_runner_provider*)context;
    rpc_status_t rpc_status = TS_RPC_ERROR_SERIALIZATION_NOT_SUPPORTED;
    struct test_spec test_spec;

    struct call_param_buf *req_buf = call_req_get_req_buf(req);
    const struct test_runner_provider_serializer *serializer = get_test_runner_serializer(this_instance, req);

    if (serializer)
        rpc_status = serializer->deserialize_list_tests_req(req_buf, &test_spec);

    if (rpc_status == TS_RPC_CALL_ACCEPTED) {

        int test_status;
        struct test_summary summary;
        size_t result_limit = 0;
        struct test_result *result_buf = alloc_result_buf(this_instance, &test_spec, &result_limit);

        test_status = run_qualifying_tests(this_instance, true, &test_spec, &summary, result_buf, result_limit);

        call_req_set_opstatus(req, test_status);

        if (test_status == TS_TEST_RUNNER_STATUS_SUCCESS) {

            struct call_param_buf *resp_buf = call_req_get_resp_buf(req);
            rpc_status = serializer->serialize_list_tests_resp(resp_buf, &summary, result_buf);

            free(result_buf);
        }
    }

    return rpc_status;
}
/**
 * \file test/endorse/test_endorse_config.cpp
 *
 * \brief Unit tests for endorse_config.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <minunit/minunit.h>
#include <string>
#include <string.h>
#include <vctool/endorse.h>
#include <vector>

extern "C" {
#include "endorse.tab.h"
#include "endorse.yy.h"
}

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/* start of the endorse_config test suite. */
TEST_SUITE(endorse_config);

/**
 * \brief Simple user context structure for testing.
 */
struct test_context
{
    resource hdr;
    rcpr_allocator* alloc;
    vector<string>* errors;
    endorse_config* config;
};

/**
 * \brief Release a test context resource.
 */
static status test_context_resource_release(resource* r)
{
    status config_release_retval = STATUS_SUCCESS;
    status release_retval = STATUS_SUCCESS;
    test_context* ctx = (test_context*)r;

    rcpr_allocator* alloc = ctx->alloc;

    if (nullptr != ctx->config)
    {
        config_release_retval = resource_release(&ctx->config->hdr);
    }

    delete ctx->errors;

    release_retval = rcpr_allocator_reclaim(alloc, ctx);

    if (STATUS_SUCCESS != config_release_retval)
    {
        return config_release_retval;
    }
    else
    {
        return release_retval;
    }
}

/**
 * \brief Initialize a test_context structure.
 */
static status test_context_create(test_context** ctx, rcpr_allocator* alloc)
{
    status retval;

    retval = rcpr_allocator_allocate(alloc, (void**)ctx, sizeof(test_context));
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    resource_init(&(*ctx)->hdr, &test_context_resource_release);
    (*ctx)->alloc = alloc;
    (*ctx)->errors = new vector<string>();
    (*ctx)->config = nullptr;

    return STATUS_SUCCESS;
}

/**
 * \brief Simple error setting override.
 */
static void set_error(endorse_config_context* context, const char* msg)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->errors->push_back(msg);
}

/**
 * \brief Simple value setting override.
 */
static void config_callback(
    endorse_config_context* context, endorse_config* config)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->config = config;
}

/**
 * Test that an empty config file produces a blank config.
 */
TEST(empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    endorse_config_context context;
    test_context* user_context;
    rcpr_allocator* alloc;

    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    TEST_ASSERT(STATUS_SUCCESS == test_context_create(&user_context, alloc));

    context.alloc = alloc;
    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context->errors->size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context->config);
    /* the entities tree sohuld not be NULL. */
    TEST_ASSERT(nullptr != user_context->config->entities);
    /* the number of entities should be zero. */
    TEST_EXPECT(0 == rbtree_count(user_context->config->entities));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that a bad config file raises an error.
 */
TEST(bad_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    endorse_config_context context;
    test_context* user_context;
    rcpr_allocator* alloc;

    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    TEST_ASSERT(STATUS_SUCCESS == test_context_create(&user_context, alloc));

    context.alloc = alloc;
    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("some garbage", scanner)));
    TEST_ASSERT(0 != yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are errors. */
    TEST_ASSERT(0U != user_context->errors->size());

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can parse an empty entities block.
 */
TEST(empty_entities_block)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    endorse_config_context context;
    test_context* user_context;
    rcpr_allocator* alloc;

    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    TEST_ASSERT(STATUS_SUCCESS == test_context_create(&user_context, alloc));

    context.alloc = alloc;
    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("entities { }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context->errors->size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context->config);
    /* the entities tree sohuld not be NULL. */
    TEST_ASSERT(nullptr != user_context->config->entities);
    /* the number of entities should be zero. */
    TEST_EXPECT(0 == rbtree_count(user_context->config->entities));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can parse an entities block with entries.
 */
TEST(entities_block)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    endorse_config_context context;
    test_context* user_context;
    rcpr_allocator* alloc;
    resource* val;

    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    TEST_ASSERT(STATUS_SUCCESS == test_context_create(&user_context, alloc));

    context.alloc = alloc;
    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != 
        (state = yy_scan_string("entities { foo bar baz }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context->errors->size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context->config);
    /* the entities tree sohuld not be NULL. */
    TEST_ASSERT(nullptr != user_context->config->entities);
    /* the number of entities should be three. */
    TEST_EXPECT(3 == rbtree_count(user_context->config->entities));
    /* we can find foo. */
    TEST_EXPECT(
        STATUS_SUCCESS ==
            rbtree_find(&val, user_context->config->entities, "foo"));
    /* we can find bar. */
    TEST_EXPECT(
        STATUS_SUCCESS ==
            rbtree_find(&val, user_context->config->entities, "bar"));
    /* we can find baz. */
    TEST_EXPECT(
        STATUS_SUCCESS ==
            rbtree_find(&val, user_context->config->entities, "baz"));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

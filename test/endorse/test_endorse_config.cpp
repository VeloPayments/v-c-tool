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
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == user_context->config->reference_count);
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
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == user_context->config->reference_count);
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
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == user_context->config->reference_count);
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

/**
 * Test that duplicate entity decls cause an error.
 */
TEST(duplicate_entity_decls)
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
    TEST_ASSERT(nullptr != 
        (state =
            yy_scan_string(
                R"MULTI(
                    entities { foo bar foo }
                )MULTI", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
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
 * Test that we can parse an empty verb block.
 */
TEST(empty_verb_block)
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
        (state = yy_scan_string("verbs for agentd { }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context->errors->size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context->config);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == user_context->config->reference_count);
    /* the entities tree sohuld not be NULL. */
    TEST_ASSERT(nullptr != user_context->config->entities);
    /* the number of entities should be three. */
    TEST_ASSERT(1 == rbtree_count(user_context->config->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, user_context->config->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was NOT declared. */
    TEST_EXPECT(!agentd->id_declared);
    /* there are no verbs defined. */
    TEST_EXPECT(0 == rbtree_count(agentd->verbs));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * If we parse a verb block for a declared entity, the id_declared flag is true.
 */
TEST(empty_verb_block_declared_entity)
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
        (state =
            yy_scan_string(
                R"MULTI(
                    entities { agentd }
                    verbs for agentd { }
                )MULTI", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context->errors->size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context->config);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == user_context->config->reference_count);
    /* the entities tree sohuld not be NULL. */
    TEST_ASSERT(nullptr != user_context->config->entities);
    /* the number of entities should be three. */
    TEST_ASSERT(1 == rbtree_count(user_context->config->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, user_context->config->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there are no verbs defined. */
    TEST_EXPECT(0 == rbtree_count(agentd->verbs));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can add verbs in a verb block.
 */
TEST(verb_block_with_verbs)
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
        (state =
            yy_scan_string(
                R"MULTI(
                verbs for agentd {
                    block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
                    transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
                    artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
                })MULTI", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context->errors->size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context->config);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == user_context->config->reference_count);
    /* the entities tree sohuld not be NULL. */
    TEST_ASSERT(nullptr != user_context->config->entities);
    /* the number of entities should be three. */
    TEST_ASSERT(1 == rbtree_count(user_context->config->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, user_context->config->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was NOT declared. */
    TEST_EXPECT(!agentd->id_declared);
    /* there are no verbs defined. */
    TEST_EXPECT(3 == rbtree_count(agentd->verbs));

    /* we can find the block_get verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, agentd->verbs, "block_get"));

    /* examine properties of block_get. */
    endorse_verb* block_get = (endorse_verb*)val;
    /* the is only referenced once. */
    TEST_EXPECT(1 == block_get->reference_count);
    /* the verb name is "block_get" */
    TEST_EXPECT(0 == strcmp(block_get->verb, "block_get"));
    /* the UUID is correct. */
    const vpr_uuid block_get_uuid = { .data = {
        0xf3, 0x82, 0xe3, 0x65, 0x12, 0x24, 0x43, 0xb4,
        0x92, 0x4a, 0x1d, 0xe4, 0xd9, 0xf4, 0xcf, 0x25 } };
    TEST_EXPECT(0 == memcmp(&block_get->verb_id, &block_get_uuid, 16));

    /* we can find the transaction_get verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, agentd->verbs, "transaction_get"));

    /* examine properties of block_get. */
    endorse_verb* transaction_get = (endorse_verb*)val;
    /* the is only referenced once. */
    TEST_EXPECT(1 == transaction_get->reference_count);
    /* the verb name is "transaction_get" */
    TEST_EXPECT(0 == strcmp(transaction_get->verb, "transaction_get"));
    /* the UUID is correct. */
    const vpr_uuid txn_get_uuid = { .data = {
        0x7d, 0xf2, 0x10, 0xd6, 0xf0, 0x0b, 0x47, 0xc4,
        0xa6, 0x08, 0x6f, 0x3f, 0x1d, 0xf7, 0x51, 0x1a } };
    TEST_EXPECT(0 == memcmp(&transaction_get->verb_id, &txn_get_uuid, 16));

    /* we can find the artifact_get verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, agentd->verbs, "artifact_get"));

    /* examine properties of block_get. */
    endorse_verb* artifact_get = (endorse_verb*)val;
    /* the is only referenced once. */
    TEST_EXPECT(1 == artifact_get->reference_count);
    /* the verb name is "artifact_get" */
    TEST_EXPECT(0 == strcmp(artifact_get->verb, "artifact_get"));
    /* the UUID is correct. */
    const vpr_uuid artifact_get_uuid = { .data = {
        0xfc, 0x0e, 0x22, 0xea, 0x1e, 0x77, 0x4e, 0xa4,
        0xa2, 0xae, 0x08, 0xbe, 0x5f, 0xf7, 0x3c, 0xcc } };
    TEST_EXPECT(0 == memcmp(&artifact_get->verb_id, &artifact_get_uuid, 16));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&user_context->hdr));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that duplicate verbs cause an error.
 */
TEST(duplicate_verbs_for_entity)
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
    TEST_ASSERT(nullptr != 
        (state =
            yy_scan_string(
                R"MULTI(
                verbs for agentd {
                    block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
                    transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
                    artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
                    block_get           64f349a9-e065-426c-b72d-276e6bf016ca
                })MULTI", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
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
 * Test that duplicate verbs cause an error.
 */
TEST(duplicate_verbs_for_entity2)
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
    TEST_ASSERT(nullptr != 
        (state =
            yy_scan_string(
                R"MULTI(
                verbs for agentd {
                    block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
                    transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
                    artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
                }

                verbs for agentd {
                    block_get           64f349a9-e065-426c-b72d-276e6bf016ca
                })MULTI", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
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

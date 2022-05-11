/**
 * \file test/endorse/test_endorse_config.cpp
 *
 * \brief Unit tests for endorse_config.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <minunit/minunit.h>
#include <string.h>
#include <vctool/endorse.h>
#include <vpr/allocator/malloc_allocator.h>

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
 * Test that an empty config file produces a blank config.
 */
TEST(empty_config)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] = "";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be zero. */
    TEST_EXPECT(0 == rbtree_count(root->entities));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that a bad config file raises an error.
 */
TEST(bad_config)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] = "some garbage";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parsing will fail. */
    TEST_ASSERT(STATUS_SUCCESS != endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can parse an empty entities block.
 */
TEST(empty_entities_block)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] = "entities { }";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be zero. */
    TEST_EXPECT(0 == rbtree_count(root->entities));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can parse an entities block with entries.
 */
TEST(entities_block)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] = "entities { foo bar baz }";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be three. */
    TEST_EXPECT(3 == rbtree_count(root->entities));

    /* we can find foo. */
    TEST_EXPECT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "foo"));
    /* we can find bar. */
    TEST_EXPECT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "bar"));
    /* we can find baz. */
    TEST_EXPECT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "baz"));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that duplicate entity decls cause an error.
 */
TEST(duplicate_entity_decls)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
            entities { foo bar foo }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are errors. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can parse an empty verb block.
 */
TEST(empty_verb_block)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] = "verbs for agentd { }";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was NOT declared. */
    TEST_EXPECT(!agentd->id_declared);
    /* there are no verbs defined. */
    TEST_EXPECT(0 == rbtree_count(agentd->verbs));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * If we parse a verb block for a declared entity, the id_declared flag is true.
 */
TEST(empty_verb_block_declared_entity)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
            entities { agentd }
            verbs for agentd { }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there are no verbs defined. */
    TEST_EXPECT(0 == rbtree_count(agentd->verbs));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can add verbs in a verb block.
 */
TEST(verb_block_with_verbs)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
            verbs for agentd {
                block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
                transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
                artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

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
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that duplicate verbs cause an error.
 */
TEST(duplicate_verbs_for_entity)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        verbs for agentd {
            block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
            artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            block_get           64f349a9-e065-426c-b72d-276e6bf016ca
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are errors. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that duplicate verbs cause an error.
 */
TEST(duplicate_verbs_for_entity2)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        verbs for agentd {
            block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
            artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
        }

        verbs for agentd {
            block_get           64f349a9-e065-426c-b72d-276e6bf016ca
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are errors. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Test that we can parse an empty roles block.
 */
TEST(empty_roles_block)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] = "roles for agentd { }";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was NOT declared. */
    TEST_EXPECT(!agentd->id_declared);
    /* there are no roles defined. */
    TEST_EXPECT(0 == rbtree_count(agentd->roles));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * If we parse a roles block for a declared entity, the id_declared flag is
 * true.
 */
TEST(empty_roles_block_declared_entity)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd { }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there are no roles defined. */
    TEST_EXPECT(0 == rbtree_count(agentd->roles));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that we can parse empty roles.
 */
TEST(empty_roles)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd {
            reader { }
            submitter { }
            extended_sentinel { }
        }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there are three roles defined. */
    TEST_EXPECT(3 == rbtree_count(agentd->roles));

    /* we can find the reader role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "reader"));

    /* examine properties of the reader role. */
    endorse_role* reader = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == reader->reference_count);
    /* the name is "reader". */
    TEST_EXPECT(!strcmp(reader->name, "reader"));
    /* no verbs are defined for reader. */
    TEST_EXPECT(0 == rbtree_count(reader->verbs));

    /* we can find the submitter role. */
    TEST_ASSERT(
        STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "submitter"));

    /* examine properties of the submitter role. */
    endorse_role* submitter = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == submitter->reference_count);
    /* the name is "submitter". */
    TEST_EXPECT(!strcmp(submitter->name, "submitter"));
    /* no verbs are defined for submitter. */
    TEST_EXPECT(0 == rbtree_count(submitter->verbs));

    /* we can find the extended_sentinel role. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == rbtree_find(&val, agentd->roles, "extended_sentinel"));

    /* examine properties of the extended_sentinel role. */
    endorse_role* extended_sentinel = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == extended_sentinel->reference_count);
    /* the name is "extended_sentinel". */
    TEST_EXPECT(!strcmp(extended_sentinel->name, "extended_sentinel"));
    /* no verbs are defined for extended_sentinel. */
    TEST_EXPECT(0 == rbtree_count(extended_sentinel->verbs));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that duplicate roles cause an error.
 */
TEST(duplicate_roles)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd {
            reader { }
            submitter { }
            extended_sentinel { }
            reader { }
        }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are errors. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that we can parse a role with verbs.
 */
TEST(role_with_verbs)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd {
            reader {
                latest_block_id_read
                next_block_id_get
                prev_block_id_get
            }
        }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id was declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there is one role defined. */
    TEST_EXPECT(1 == rbtree_count(agentd->roles));

    /* we can find the reader role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "reader"));

    /* examine properties of the reader role. */
    endorse_role* reader = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == reader->reference_count);
    /* the name is "reader". */
    TEST_EXPECT(!strcmp(reader->name, "reader"));
    /* there are three verbs defined for the reader role. */
    TEST_EXPECT(3 == rbtree_count(reader->verbs));

    /* we can find the latest_block_id_read verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, reader->verbs, "latest_block_id_read"));

    /* examine properties of the latest_block_id_read role_verb. */
    endorse_role_verb* latest_block_id_read = (endorse_role_verb*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == latest_block_id_read->reference_count);
    /* the verb name is "latest_block_id_read". */
    TEST_EXPECT(
        !strcmp(latest_block_id_read->verb_name, "latest_block_id_read"));
    /* the verb reference is not set. */
    TEST_EXPECT(NULL == latest_block_id_read->verb);

    /* we can find the next_block_id_get verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, reader->verbs, "next_block_id_get"));

    /* examine properties of the next_block_id_get role_verb. */
    endorse_role_verb* next_block_id_get = (endorse_role_verb*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == next_block_id_get->reference_count);
    /* the verb name is "next_block_id_get". */
    TEST_EXPECT(
        !strcmp(next_block_id_get->verb_name, "next_block_id_get"));
    /* the verb reference is not set. */
    TEST_EXPECT(NULL == next_block_id_get->verb);

    /* we can find the prev_block_id_get verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, reader->verbs, "prev_block_id_get"));

    /* examine properties of the prev_block_id_get role_verb. */
    endorse_role_verb* prev_block_id_get = (endorse_role_verb*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == prev_block_id_get->reference_count);
    /* the verb name is "prev_block_id_get". */
    TEST_EXPECT(
        !strcmp(prev_block_id_get->verb_name, "prev_block_id_get"));
    /* the verb reference is not set. */
    TEST_EXPECT(NULL == prev_block_id_get->verb);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that duplicate role verbs cause an error.
 */
TEST(duplicate_role_verbs)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd {
            reader {
                latest_block_id_read
                next_block_id_get
                prev_block_id_get
                latest_block_id_read
            }
        }
        )MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are errors. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that an undeclared entity raises a semantic error.
 */
TEST(undeclared_entity_semantic_error)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        verbs for agentd {
            block_get           f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get     7df210d6-f00b-47c4-a608-6f3f1df7511a
            artifact_get        fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* get the endorse config root. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);

    /* perform the semantic analysis on this config context. */
    TEST_ASSERT(
        STATUS_SUCCESS != endorse_analyze(ctx, (endorse_config*)root));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that roles with undeclared verbs raise errors in semantic analysis.
 */
TEST(undefined_verb_semantic_error)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd {
            reader {
                document_get
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* get the root config. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* perform the semantic analysis on this config context. */
    TEST_ASSERT(
        STATUS_SUCCESS != endorse_analyze(ctx, root));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * Verify that in a successful parse, the role's verbs are populated with the
 * real verb references.
 */
TEST(semantic_analyzer_populates_verb_references)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        verbs for agentd {
            latest_block_id_get     c5b0eb04-6b24-48be-b7d9-bf9083a4be5d
        }
        roles for agentd {
            reader {
                latest_block_id_get
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* get the root config. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* perform the semantic analysis on this config context. */
    TEST_ASSERT(
        STATUS_SUCCESS == endorse_analyze(ctx, root));

    /* verify user config. */
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_ASSERT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id WAS declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there is one verb defined. */
    TEST_EXPECT(1 == rbtree_count(agentd->verbs));
    /* there is one role defined. */
    TEST_EXPECT(1 == rbtree_count(agentd->roles));

    /* we can find the latest_block_id_get verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, agentd->verbs, "latest_block_id_get"));

    /* examine properties of latest_block_id_get. */
    endorse_verb* latest_block_id_get = (endorse_verb*)val;
    /* it is referenced TWICE. */
    TEST_EXPECT(2 == latest_block_id_get->reference_count);
    /* the verb name is "latest_block_id_get" */
    TEST_EXPECT(0 == strcmp(latest_block_id_get->verb, "latest_block_id_get"));
    /* the UUID is correct. */
    const vpr_uuid latest_block_id_get_uuid = { .data = {
        0xc5, 0xb0, 0xeb, 0x04, 0x6b, 0x24, 0x48, 0xbe,
        0xb7, 0xd9, 0xbf, 0x90, 0x83, 0xa4, 0xbe, 0x5d } };
    TEST_EXPECT(
        0 ==
            memcmp(
                &latest_block_id_get->verb_id, &latest_block_id_get_uuid, 16));

    /* we can find the reader role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "reader"));

    /* examine properties of the reader role. */
    endorse_role* reader = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == reader->reference_count);
    /* the name is "reader". */
    TEST_EXPECT(!strcmp(reader->name, "reader"));
    /* one verb is defined for reader. */
    TEST_EXPECT(1 == rbtree_count(reader->verbs));

    /* we can find the latest_block_id_get role verb. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, reader->verbs, "latest_block_id_get"));

    /* examine properties of latest_block_id_get. */
    endorse_role_verb* rv_latest_block_id_get = (endorse_role_verb*)val;
    /* it is referenced once. */
    TEST_EXPECT(1 == rv_latest_block_id_get->reference_count);
    /* the verb name is "latest_block_id_get" */
    TEST_EXPECT(
        0 == strcmp(rv_latest_block_id_get->verb_name, "latest_block_id_get"));
    /* the semantic analyzer added the real verb reference. */
    TEST_EXPECT(
        latest_block_id_get == rv_latest_block_id_get->verb);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * A role can inherit another role via the extends keyword.
 */
TEST(role_extends)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        roles for agentd {
            reader {
                latest_block_id_get
            }
            writer extends reader {
                transaction_submit
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    const endorse_config* root =
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);
    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id WAS declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there are two roles defined. */
    TEST_EXPECT(2 == rbtree_count(agentd->roles));

    /* we can find the reader role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "reader"));

    /* examine properties of the reader role. */
    endorse_role* reader = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == reader->reference_count);
    /* the name is "reader". */
    TEST_EXPECT(!strcmp(reader->name, "reader"));
    /* one verb is defined for reader. */
    TEST_EXPECT(1 == rbtree_count(reader->verbs));
    /* the reader role does not extend any other role. */
    TEST_EXPECT(NULL == reader->extends_role_name);

    /* we can find the writer role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "writer"));

    /* examine properties of the writer role. */
    endorse_role* writer = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == writer->reference_count);
    /* the name is "writer". */
    TEST_EXPECT(!strcmp(writer->name, "writer"));
    /* one verb is defined for writer. */
    TEST_EXPECT(1 == rbtree_count(writer->verbs));
    /* the writer role extends the reader role. */
    TEST_ASSERT(NULL != writer->extends_role_name);
    TEST_EXPECT(0 == strcmp(writer->extends_role_name, "reader"));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * During semantic analysis, role inheritance populates the inherited role as
 * tell as all verbs supported by the role.
 */
TEST(role_extends_semantic_analysis)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    resource* val;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        verbs for agentd {
            latest_block_id_get     c5b0eb04-6b24-48be-b7d9-bf9083a4be5d
            next_block_id_get       6a399f0d-ddb3-45dc-b2e3-0227a962b237
            prev_block_id_get       73cfae64-80e8-412d-b005-344d537766a6
            block_get               f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get         7df210d6-f00b-47c4-a608-6f3f1df7511a
            transaction_submit      ef560d24-eea6-4847-9009-464b127f249b
            artifact_get            fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            assert_latest_block_id  447617b4-a847-437c-b62b-5bc6a94206fa
            sentinel_extend_api     c41b053c-6b4a-40a1-981b-882bdeffe978
        }
        roles for agentd {
            reader {
                latest_block_id_get
                block_get
            }
            writer extends reader {
                transaction_submit
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* perform the semantic analysis on this config context. */
    TEST_ASSERT(
        STATUS_SUCCESS == endorse_analyze(ctx, root));

    /* There should only be one reference to this config. */
    TEST_EXPECT(1 == root->reference_count);
    /* the entities tree should not be NULL. */
    TEST_ASSERT(nullptr != root->entities);
    /* the number of entities should be one. */
    TEST_EXPECT(1 == rbtree_count(root->entities));

    /* we can find agentd. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            rbtree_find(&val, root->entities, "agentd"));

    /* examine properties of agentd. */
    endorse_entity* agentd = (endorse_entity*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == agentd->reference_count);
    /* the id WAS declared. */
    TEST_EXPECT(agentd->id_declared);
    /* there are nine verbs defined. */
    TEST_EXPECT(9 == rbtree_count(agentd->verbs));
    /* there are two roles defined. */
    TEST_EXPECT(2 == rbtree_count(agentd->roles));

    /* we can find the reader role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "reader"));

    /* examine properties of the reader role. */
    endorse_role* reader = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(2 == reader->reference_count);
    /* the name is "reader". */
    TEST_EXPECT(!strcmp(reader->name, "reader"));
    /* two verbs are defined for reader. */
    TEST_EXPECT(2 == rbtree_count(reader->verbs));
    /* the reader role does not extend any other role. */
    TEST_EXPECT(NULL == reader->extends_role_name);
    /* the reader role extends_role pointer is NULL. */
    TEST_EXPECT(NULL == reader->extends_role);

    /* we can find the writer role. */
    TEST_ASSERT(STATUS_SUCCESS == rbtree_find(&val, agentd->roles, "writer"));

    /* examine properties of the writer role. */
    endorse_role* writer = (endorse_role*)val;
    /* this is only referenced once. */
    TEST_EXPECT(1 == writer->reference_count);
    /* the name is "writer". */
    TEST_EXPECT(!strcmp(writer->name, "writer"));
    /* THREE verbs are defined for writer. */
    TEST_EXPECT(3 == rbtree_count(writer->verbs));
    /* the writer role extends the reader role. */
    TEST_ASSERT(NULL != writer->extends_role_name);
    TEST_EXPECT(0 == strcmp(writer->extends_role_name, "reader"));
    /* the extended role is the reader role. */
    TEST_EXPECT(reader == writer->extends_role);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * During semantic analysis, a circular role inheritance graph causes an error.
 * Simple circle - role references itself.
 */
TEST(role_extends_analysis_circular_n0)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        verbs for agentd {
            latest_block_id_get     c5b0eb04-6b24-48be-b7d9-bf9083a4be5d
            next_block_id_get       6a399f0d-ddb3-45dc-b2e3-0227a962b237
            prev_block_id_get       73cfae64-80e8-412d-b005-344d537766a6
            block_get               f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get         7df210d6-f00b-47c4-a608-6f3f1df7511a
            transaction_submit      ef560d24-eea6-4847-9009-464b127f249b
            artifact_get            fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            assert_latest_block_id  447617b4-a847-437c-b62b-5bc6a94206fa
            sentinel_extend_api     c41b053c-6b4a-40a1-981b-882bdeffe978
        }
        roles for agentd {
            reader {
                latest_block_id_get
                block_get
            }
            writer extends writer {
                transaction_submit
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* Semantic analysis fails due to circular reference. */
    TEST_ASSERT(
        STATUS_SUCCESS != endorse_analyze(ctx, root));

    /* errors were set. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * During semantic analysis, a circular role inheritance graph causes an error.
 * N1 circle.
 */
TEST(role_extends_analysis_circular_n1)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        verbs for agentd {
            latest_block_id_get     c5b0eb04-6b24-48be-b7d9-bf9083a4be5d
            next_block_id_get       6a399f0d-ddb3-45dc-b2e3-0227a962b237
            prev_block_id_get       73cfae64-80e8-412d-b005-344d537766a6
            block_get               f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get         7df210d6-f00b-47c4-a608-6f3f1df7511a
            transaction_submit      ef560d24-eea6-4847-9009-464b127f249b
            artifact_get            fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            assert_latest_block_id  447617b4-a847-437c-b62b-5bc6a94206fa
            sentinel_extend_api     c41b053c-6b4a-40a1-981b-882bdeffe978
        }
        roles for agentd {
            reader extends writer {
                latest_block_id_get
                block_get
            }
            writer extends reader {
                transaction_submit
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* get root. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* Semantic analysis fails due to circular reference. */
    TEST_ASSERT(
        STATUS_SUCCESS != endorse_analyze(ctx, root));

    /* errors were set. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * During semantic analysis, a circular role inheritance graph causes an error.
 * N2 circle.
 */
TEST(role_extends_analysis_circular_n2)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        verbs for agentd {
            latest_block_id_get     c5b0eb04-6b24-48be-b7d9-bf9083a4be5d
            next_block_id_get       6a399f0d-ddb3-45dc-b2e3-0227a962b237
            prev_block_id_get       73cfae64-80e8-412d-b005-344d537766a6
            block_get               f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get         7df210d6-f00b-47c4-a608-6f3f1df7511a
            transaction_submit      ef560d24-eea6-4847-9009-464b127f249b
            artifact_get            fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            assert_latest_block_id  447617b4-a847-437c-b62b-5bc6a94206fa
            sentinel_extend_api     c41b053c-6b4a-40a1-981b-882bdeffe978
        }
        roles for agentd {
            a extends b {
            }
            b extends c {
            }
            c extends a {
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* get the root node. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* Semantic analysis fails due to circular reference. */
    TEST_ASSERT(
        STATUS_SUCCESS != endorse_analyze(ctx, root));

    /* errors were set. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * During semantic analysis, a circular role inheritance graph causes an error.
 * N3 circle.
 */
TEST(role_extends_analysis_circular_n3)
{
    endorse_config_context* ctx;
    rcpr_allocator* alloc;
    allocator_options_t vpr_alloc;
    vccrypt_buffer_t input;
    const char INPUT[] =
        R"MULTI(
        entities {
            agentd
        }
        verbs for agentd {
            latest_block_id_get     c5b0eb04-6b24-48be-b7d9-bf9083a4be5d
            next_block_id_get       6a399f0d-ddb3-45dc-b2e3-0227a962b237
            prev_block_id_get       73cfae64-80e8-412d-b005-344d537766a6
            block_get               f382e365-1224-43b4-924a-1de4d9f4cf25
            transaction_get         7df210d6-f00b-47c4-a608-6f3f1df7511a
            transaction_submit      ef560d24-eea6-4847-9009-464b127f249b
            artifact_get            fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc
            assert_latest_block_id  447617b4-a847-437c-b62b-5bc6a94206fa
            sentinel_extend_api     c41b053c-6b4a-40a1-981b-882bdeffe978
        }
        roles for agentd {
            a extends b {
            }
            b extends c {
            }
            c extends d {
            }
            d extends a {
            }
        })MULTI";

    /* create the RCPR malloc allocator. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create the VPR malloc allocator. */
    malloc_allocator_options_init(&vpr_alloc);

    /* create a default config context. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_config_create_default(&ctx, alloc));

    /* create a buffer with our string. */
    TEST_ASSERT(
        STATUS_SUCCESS ==
            vccrypt_buffer_init(&input, &vpr_alloc, strlen(INPUT) + 1));
    memset(input.data, 0, input.size);
    TEST_ASSERT(
        STATUS_SUCCESS == vccrypt_buffer_read_data(&input, INPUT, input.size));

    /* parse the data. */
    TEST_ASSERT(STATUS_SUCCESS == endorse_parse(ctx, &input));

    /* there are no errors. */
    TEST_ASSERT(
        0U == endorse_config_default_context_get_error_message_count(ctx));

    /* verify user config. */
    endorse_config* root = (endorse_config*)
        endorse_config_default_context_get_endorse_config_root(ctx);
    TEST_ASSERT(nullptr != root);

    /* Semantic analysis fails due to circular reference. */
    TEST_ASSERT(
        STATUS_SUCCESS != endorse_analyze(ctx, root));

    /* errors were set. */
    TEST_ASSERT(
        0U != endorse_config_default_context_get_error_message_count(ctx));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == resource_release(&ctx->hdr));
    dispose(vccrypt_buffer_disposable_handle(&input));
    dispose(allocator_options_disposable_handle(&vpr_alloc));
    TEST_ASSERT(
        STATUS_SUCCESS ==
            resource_release(rcpr_allocator_resource_handle(alloc)));
}

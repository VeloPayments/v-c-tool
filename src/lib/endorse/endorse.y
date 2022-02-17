/**
 * \file lib/endorse/endorse.y
 *
 * \brief Parser for vctool endorse config files.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

%{
#include <string.h>
#include <rcpr/compare.h>
#include <vctool/endorse.h>
#include <vpr/parameters.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_compare;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Helper macro for passing an error condition to the caller and breaking
 * out of the parse.
 */
#define CONFIG_ERROR(s) \
    context->set_error(context, (s)); \
    return NULL

/**
 * \brief Helper macro for breaking out of the parse if a NULL pointer is
 * returned by the helper method.
 */
#define MAYBE_ASSIGN(lhs, rhs) \
    do { \
        typeof (lhs) x = (rhs); \
        if (NULL == x) YYACCEPT; \
        (lhs) = x; \
    } while (false)

/* forward decls */
int yylex();
int yyerror(
    yyscan_t scanner, endorse_config_context* context, const char*);
static endorse_config* new_endorse_config(endorse_config_context* context);
static endorse_config* merge_entities(
    endorse_config_context*, endorse_config*, rbtree*);
static status endorse_config_resource_release(resource* r);
static rcpr_comparison_result endorse_entities_compare(
    void*, const void* lhs, const void* rhs);
static const void* endorse_entities_key(
    void*, const resource* r);
static rbtree* new_entities(endorse_config_context*);
static rbtree* add_entity(endorse_config_context*, rbtree*, const char*, bool);
static status entity_resource_release(resource* r);
%}

/* use the full pure API for Bison. */
%define api.pure full
/* We pass the scanner to the lexer. */
%lex-param {yyscan_t scanner}
/* We pass the scanner to Bison. */
%parse-param {yyscan_t scanner}
/* We pass our config context structure to Bison. */
%parse-param {endorse_config_context* context}

/* Tokens. */
%token <string> COMMA
%token <string> LBRACE
%token <string> ENTITIES
%token <string> EXTENDS
%token <string> FOR
%token <string> IDENTIFIER
%token <string> INVALID
%token <string> RBRACE
%token <string> ROLES
%token <id> UUID
%token <id> UUID_INVALID
%token <string> VERBS

%type <config> endorse
%type <entities> entities;
%type <entities> entities_block;

%destructor { resource_release(&$$->hdr); } <config>
%destructor { resource_release(rbtree_resource_handle($$)); } <entities>

%%

/* Base endorse config rule. */
endorse : {
            /* create a new config. */
            MAYBE_ASSIGN($$, new_endorse_config(context));
            /* increment reference count. */
            ++($$->reference_count);
            context->val_callback(context, $$); }
    | endorse entities {
            /* fold in entities. */
            MAYBE_ASSIGN($$, merge_entities(context, $1, $2)); }
    ;

entities
    : ENTITIES LBRACE entities_block RBRACE {
        /* ownership is forwarded. */
        $$ = $3; }
    ;

entities_block
    : {
        /* create a new entities block. */
        MAYBE_ASSIGN($$, new_entities(context)); }
    | entities_block IDENTIFIER {
        /* add an entity to the entities tree. */
        MAYBE_ASSIGN($$, add_entity(context, $1, $2, true)); }
    ;
%%

/**
 * \brief Create a new configuration structure.
 */
static endorse_config* new_endorse_config(endorse_config_context* context)
{
    status retval, release_retval;
    const char* error_message = NULL;
    endorse_config* cfg = NULL;

    retval =
        rcpr_allocator_allocate(context->alloc, (void**)&cfg, sizeof(*cfg));
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory in new_endorse_config().";
        goto error_exit;
    }

    /* set up config structure. */
    memset(cfg, 0, sizeof(*cfg));
    resource_init(&cfg->hdr, &endorse_config_resource_release);
    cfg->alloc = context->alloc;
    cfg->reference_count = 1;

    /* create an entities rbtree. */
    cfg->entities = new_entities(context);
    if (NULL == cfg->entities)
    {
        error_message = "Out of memory creating entities tree.";
        goto cleanup_cfg;
    }

    return cfg;

cleanup_cfg:
    release_retval = resource_release(&cfg->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Release an endorse config resource.
 */
static status endorse_config_resource_release(resource* r)
{
    status entities_release_retval = STATUS_SUCCESS;
    status reclaim_retval = STATUS_SUCCESS;
    endorse_config* cfg = (endorse_config*)r;

    /* decrement reference count. */
    --cfg->reference_count;

    /* if there are still references, don't release this resource. */
    if (cfg->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    rcpr_allocator* alloc = cfg->alloc;

    /* release the entities resource. */
    if (NULL != cfg->entities)
    {
        entities_release_retval =
            resource_release(rbtree_resource_handle(cfg->entities));
    }

    /* clear memory. */
    memset(cfg, 0, sizeof(*cfg));

    /* reclaim memory. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, cfg);

    /* decode the return value. */
    if (STATUS_SUCCESS != entities_release_retval)
    {
        return entities_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}

/**
 * \brief Compare two entities by key.
 */
static rcpr_comparison_result endorse_entities_compare(
    void*, const void* lhs, const void* rhs)
{
    const char* l = (const char*)lhs;
    const char* r = (const char*)rhs;

    int result = strcmp(l, r);
    if (result < 0)
    {
        return RCPR_COMPARE_LT;
    }
    else if (result > 0)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}

/**
 * \brief Get the key for an endorse config entity.
 */
static const void* endorse_entities_key(
    void*, const resource* r)
{
    const endorse_entity* entity = (const endorse_entity*)r;

    return (const void*)entity->id;
}

/**
 * \brief Merge an entities tree into the config.
 */
static endorse_config* merge_entities(
    endorse_config_context* context, endorse_config* cfg, rbtree* entities)
{
    status retval;
    rbtree_node* node;
    endorse_entity* value;
    resource* value_resource;

    while (rbtree_count(entities) > 0)
    {
        node = rbtree_root_node(entities);
        value = (endorse_entity*)rbtree_node_value(entities, node);

        /* delete this node from the entities tree. */
        retval = rbtree_delete(&value_resource, entities, value->id);
        if (STATUS_SUCCESS != retval)
        {
            CONFIG_ERROR("Error deleting rbtree node during merge_entities.");
        }

        /* insert this node into the config entities tree. */
        retval = rbtree_insert(cfg->entities, value_resource);
        if (STATUS_SUCCESS != retval)
        {
            CONFIG_ERROR("Error inserting rbtree node during merge_entities.");
        }
    }

    /* success. */
    return cfg;
}

/**
 * \brief Create a new entities tree.
 */
static rbtree* new_entities(endorse_config_context* context)
{
    status retval;
    rbtree* entities;

    /* create an entities rbtree. */
    retval =
        rbtree_create(
            &entities, context->alloc, &endorse_entities_compare,
            &endorse_entities_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory creating entities tree.");
    }

    return entities;
}

/**
 * \brief Add an entity to the entities tree.
 */
static rbtree* add_entity(
    endorse_config_context* context, rbtree* entities, const char* id,
    bool is_decl)
{
    status retval;
    endorse_entity* entity;

    /* allocate an endorse_entity instance. */
    retval =
        rcpr_allocator_allocate(
            context->alloc, (void**)&entity, sizeof(*entity));
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory creating entity in add_entity.");
    }

    /* set up entity. */
    memset(entity, 0, sizeof(*entity));
    resource_init(&entity->hdr, &entity_resource_release);
    entity->alloc = context->alloc;
    entity->id = strdup(id);
    entity->reference_count = 1;
    entity->id_declared = is_decl;

    /* insert this entity into the rbtree. */
    retval = rbtree_insert(entities, &entity->hdr);
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory inserting entity in add_entity.");
    }

    /* success. */
    return entities;
}

/**
 * \brief Release an entity resource.
 */
static status entity_resource_release(resource* r)
{
    endorse_entity* entity = (endorse_entity*)r;

    /* decrement reference count. */
    --entity->reference_count;

    /* if there are still references, don't release this resource. */
    if (entity->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache allocator. */
    rcpr_allocator* alloc = entity->alloc;

    /* free the id string. */
    free((void*)entity->id);

    /* clear memory. */
    memset(entity, 0, sizeof(*entity));

    /* reclaim entity. */
    return rcpr_allocator_reclaim(alloc, entity);
}

/**
 * \brief Set the error for the config structure.
 */
int yyerror(
    yyscan_t /*scanner*/, endorse_config_context* context, const char* msg)
{
    context->set_error(context, msg);

    return 1;
}

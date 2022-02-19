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
static rcpr_comparison_result endorse_verbs_compare(
    void*, const void* lhs, const void* rhs);
static const void* endorse_verbs_key(
    void*, const resource* r);
static rbtree* new_entities(endorse_config_context*);
static rbtree* add_entity(endorse_config_context*, rbtree*, const char*, bool);
static endorse_entity* new_entity(
    endorse_config_context*, const char*, rbtree*, bool);
static status entity_resource_release(resource* r);
static rbtree* add_verb(
    endorse_config_context*, rbtree*, const char*, const vpr_uuid*);
static rbtree* new_verbs(endorse_config_context*);
static endorse_verb* new_verb(
    endorse_config_context*, const char*, const vpr_uuid*);
static status verb_resource_release(resource* r);
static endorse_config* merge_verb_entity(
    endorse_config_context*, endorse_config*, endorse_entity*);
static status merge_verbs(endorse_entity*, endorse_entity*);
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
%token COMMA
%token LBRACE
%token ENTITIES
%token EXTENDS
%token FOR
%token <string> IDENTIFIER
%token <string> INVALID
%token RBRACE
%token ROLES
%token <id> UUID
%token <id> UUID_INVALID
%token VERBS

%type <config> endorse
%type <entities> entities;
%type <entities> entities_block;
%type <entity> verb_entity;
%type <verbs> verbs_block;

%destructor { resource_release(&$$->hdr); } <config>
%destructor { resource_release(rbtree_resource_handle($$)); } <entities>
%destructor { resource_release(&$$->hdr); } <entity>
%destructor { memset($$, 0, sizeof(*$$)); free($$); } <id>
%destructor { free($$); } <string>
%destructor { resource_release(rbtree_resource_handle($$)); } <verbs>

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
    | endorse verb_entity {
            /* fold in verbs. */
            MAYBE_ASSIGN($$, merge_verb_entity(context, $1, $2)); }
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

verb_entity
    : VERBS FOR IDENTIFIER LBRACE verbs_block RBRACE {
        /* create an entity reference. */
        MAYBE_ASSIGN($$, new_entity(context, $3, $5, false)); }
    ;

verbs_block
    : {
        /* create a new verbs block. */
        MAYBE_ASSIGN($$, new_verbs(context)); }
    | verbs_block IDENTIFIER UUID {
        MAYBE_ASSIGN($$, add_verb(context, $1, $2, $3)); }
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
 * \brief Compare two verbs by key.
 */
static rcpr_comparison_result endorse_verbs_compare(
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
 * \brief Get the key for an endorse config verb.
 */
static const void* endorse_verbs_key(
    void*, const resource* r)
{
    const endorse_verb* verb = (const endorse_verb*)r;

    return (const void*)verb->verb;
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
 * \brief Create a new verbs tree.
 */
static rbtree* new_verbs(endorse_config_context* context)
{
    status retval;
    rbtree* verbs;

    /* create a verbs rbtree. */
    retval =
        rbtree_create(
            &verbs, context->alloc, &endorse_verbs_compare,
            &endorse_verbs_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory creating verbs tree.");
    }

    return verbs;
}

/**
 * \brief Add an entity to the entities tree.
 */
static rbtree* add_entity(
    endorse_config_context* context, rbtree* entities, const char* id,
    bool is_decl)
{
    status retval;
    endorse_entity* entity = NULL;
    rbtree* verbs = NULL;
    const char* error_message;

    /* create the verbs tree for this entity. */
    verbs = new_verbs(context);
    if (NULL == verbs)
    {
        error_message = "Out of memory creating entity verb tree in add_entity";
        goto error_exit;
    }

    /* create an entity. */
    entity = new_entity(context, id, verbs, is_decl);
    if (NULL == entity)
    {
        error_message = "Out of memory creating entity in add_entity.";
        goto cleanup_verbs;
    }

    /* verbs is now owned by entity. */
    verbs = NULL;

    /* insert this entity into the rbtree. */
    retval = rbtree_insert(entities, &entity->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory inserting entity in add_entity.";
        goto cleanup_entity;
    }

    /* success. */
    return entities;

cleanup_entity:
    retval = resource_release(&entity->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Error releasing entity resource.";
    }

cleanup_verbs:
    if (NULL != verbs)
    {
        retval = resource_release(rbtree_resource_handle(verbs));
        if (STATUS_SUCCESS != retval)
        {
            error_message = "Error releasing verbs resource.";
        }
        verbs = NULL;
    }

error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Create a new entity from an identifier, a verbs rbtree, and a
 * declaration flag.
 */
static endorse_entity* new_entity(
    endorse_config_context* context, const char* id, rbtree* verbs,
    bool is_decl)
{
    status retval;
    endorse_entity* entity;
    const char* error_message;

    /* allocate an endorse_entity instance. */
    retval =
        rcpr_allocator_allocate(
            context->alloc, (void**)&entity, sizeof(*entity));
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory creating entity in new_entity.";
        goto error_exit;
    }

    /* set up entity. */
    memset(entity, 0, sizeof(*entity));
    resource_init(&entity->hdr, &entity_resource_release);
    entity->alloc = context->alloc;
    entity->id = strdup(id);
    entity->reference_count = 1;
    entity->id_declared = is_decl;
    entity->verbs = verbs;

    /* success. */
    return entity;

error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Release an entity resource.
 */
static status entity_resource_release(resource* r)
{
    status reclaim_retval = STATUS_SUCCESS;
    status verbs_release_retval = STATUS_SUCCESS;
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

    /* release the verbs rbtree if set. */
    verbs_release_retval =
        resource_release(rbtree_resource_handle(entity->verbs));

    /* clear memory. */
    memset(entity, 0, sizeof(*entity));

    /* reclaim entity. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, entity);

    /* decode return status. */
    if (STATUS_SUCCESS != verbs_release_retval)
    {
        return verbs_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}

/**
 * \brief Add a verb to the verbs tree.
 */
static rbtree* add_verb(
    endorse_config_context* context, rbtree* verbs, const char* verb_name,
    const vpr_uuid* verb_id)
{
    status retval;
    endorse_verb* verb = NULL;
    const char* error_message;

    /* create a verb. */
    verb = new_verb(context, verb_name, verb_id);
    if (NULL == verb)
    {
        error_message = "Out of memory creating verb in add_verb.";
        goto error_exit;
    }

    /* insert this verby into the rbtree. */
    retval = rbtree_insert(verbs, &verb->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory inserting verb in add_verb.";
        goto cleanup_verb;
    }

    /* success. */
    return verbs;

cleanup_verb:
    retval = resource_release(&verb->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Error releasing verb resource.";
    }

error_exit:
    CONFIG_ERROR(error_message);
}

static endorse_verb* new_verb(
    endorse_config_context* context, const char* verb_name,
    const vpr_uuid* verb_id)
{
    status retval;
    endorse_verb* verb;
    const char* error_message;

    /* allocate an endorse_verb instance. */
    retval =
        rcpr_allocator_allocate(
            context->alloc, (void**)&verb, sizeof(*verb));
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory creating verb in new_verb.";
        goto error_exit;
    }

    /* set up verb. */
    memset(verb, 0, sizeof(*verb));
    resource_init(&verb->hdr, &verb_resource_release);
    verb->alloc = context->alloc;
    verb->verb = strdup(verb_name);
    memcpy(&verb->verb_id, verb_id, sizeof(verb->verb_id));
    verb->reference_count = 1;

    /* success. */
    return verb;

error_exit:
    CONFIG_ERROR(error_message);

}

static status verb_resource_release(resource* r)
{
    endorse_verb* verb = (endorse_verb*)r;

    /* decrement reference count. */
    --verb->reference_count;

    /* if there are still references, don't release this resource. */
    if (verb->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache allocator. */
    rcpr_allocator* alloc = verb->alloc;

    /* free the verb name string. */
    free((void*)verb->verb);

    /* clear memory. */
    memset(verb, 0, sizeof(*verb));

    /* reclaim verb. */
    return
        rcpr_allocator_reclaim(alloc, verb);
}

/**
 * \brief Merge an entity with verbs with the canonical entity.
 */
static endorse_config* merge_verb_entity(
    endorse_config_context* context, endorse_config* cfg,
    endorse_entity* entity)
{
    status retval;
    resource* entity_resource;

    /* attempt to look up the canonical entity by name. */
    retval = rbtree_find(&entity_resource, cfg->entities, entity->id);
    if (STATUS_SUCCESS == retval)
    {
        /* entity found; merge verbs. */
        retval = merge_verbs((endorse_entity*)entity_resource, entity);
        if (STATUS_SUCCESS != retval)
        {
            CONFIG_ERROR("Error merging verbs into the canonical entity.");
        }
    }
    else
    {
        /* this is an error case, but we will pick it up in semantic analysis.*/
        /* for now, move the entity into the canonical entity tree. */

        /* insert the entity into the entities tree. */
        retval = rbtree_insert(cfg->entities, &entity->hdr);
        if (STATUS_SUCCESS != retval)
        {
            CONFIG_ERROR("Could not merge non-canonical entity.");
        }
    }

    /* success. */
    return cfg;
}

/**
 * \brief Merge verbs into the canonical entity.
 */
static status merge_verbs(
    endorse_entity* canonical_entity, endorse_entity* entity)
{
    status retval;
    rbtree_node* node;
    endorse_verb* verb;
    resource* verb_resource;

    while (rbtree_count(entity->verbs) > 0)
    {
        node = rbtree_root_node(entity->verbs);
        verb = (endorse_verb*)rbtree_node_value(entity->verbs, node);

        /* delete this node from the entity node tree. */
        retval = rbtree_delete(&verb_resource, entity->verbs, verb->verb);
        if (STATUS_SUCCESS != retval)
        {
            return retval;
        }

        /* insert this verb into the canonical entity's verb tree. */
        retval = rbtree_insert(canonical_entity->verbs, verb_resource);
        if (STATUS_SUCCESS != retval)
        {
            return retval;
        }
    }

    /* success. */
    return STATUS_SUCCESS;
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

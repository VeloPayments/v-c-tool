/**
 * \file lib/endorse/endorse.y
 *
 * \brief Parser for vctool endorse config files.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

%{
#include <stdio.h>
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
static rcpr_comparison_result endorse_roles_compare(
    void*, const void* lhs, const void* rhs);
static const void* endorse_roles_key(
    void*, const resource* r);
static rbtree* new_entities(endorse_config_context*);
static rbtree* add_entity(endorse_config_context*, rbtree*, const char*, bool);
static endorse_entity* new_entity(
    endorse_config_context*, const char*, rbtree*, rbtree*, bool);
static status entity_resource_release(resource* r);
static rbtree* add_verb(
    endorse_config_context*, rbtree*, const char*, const vpr_uuid*);
static rbtree* new_verbs(endorse_config_context*);
static rbtree* new_roles(endorse_config_context*);
static endorse_verb* new_verb(
    endorse_config_context*, const char*, const vpr_uuid*);
static status verb_resource_release(resource* r);
static endorse_config* merge_verb_entity(
    endorse_config_context*, endorse_config*, endorse_entity*);
static status merge_verbs(
    endorse_config_context* context, endorse_entity* canonical_entity,
    endorse_entity* entity);
static endorse_config* merge_role_entity(
    endorse_config_context*, endorse_config*, endorse_entity*);
static status merge_roles(
    endorse_config_context* context, endorse_entity* canonical_entity,
    endorse_entity* entity);
static rbtree* new_role_verbs(endorse_config_context*);
static rcpr_comparison_result endorse_role_verbs_compare(
    void*, const void*, const void*);
static const void* endorse_role_verbs_key(void*, const resource*);
static rbtree* add_role(
    endorse_config_context*, rbtree*, const char*, const char*, rbtree*);
static endorse_role* new_role(
    endorse_config_context*, const char*, const char*, rbtree*);
static status role_resource_release(resource* r);
static rbtree* add_role_verb(endorse_config_context*, rbtree*, const char*);
static endorse_role_verb* new_role_verb(
    endorse_config_context*, const char*, endorse_verb*);
static status role_verb_resource_release(resource* r);
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
%type <entity> role_entity;
%type <verbs> verbs_block;
%type <roles> roles_block;
%type <role_verbs> role_verbs;

%destructor { resource_release(&$$->hdr); } <config>
%destructor { resource_release(rbtree_resource_handle($$)); } <entities>
%destructor { resource_release(&$$->hdr); } <entity>
%destructor { memset($$, 0, sizeof(*$$)); free($$); } <id>
%destructor { free($$); } <string>
%destructor { resource_release(rbtree_resource_handle($$)); } <verbs>
%destructor { resource_release(rbtree_resource_handle($$)); } <roles>
%destructor { resource_release(rbtree_resource_handle($$)); } <role_verbs>

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
    | endorse role_entity {
            /* fold in roles. */
            MAYBE_ASSIGN($$, merge_role_entity(context, $1, $2)); }
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
        MAYBE_ASSIGN($$, new_entity(context, $3, $5, NULL, false)); }
    ;

verbs_block
    : {
        /* create a new verbs block. */
        MAYBE_ASSIGN($$, new_verbs(context)); }
    | verbs_block IDENTIFIER UUID {
        MAYBE_ASSIGN($$, add_verb(context, $1, $2, $3)); }
    ;

role_entity
    : ROLES FOR IDENTIFIER LBRACE roles_block RBRACE {
        /* create an entity reference. */
        MAYBE_ASSIGN($$, new_entity(context, $3, NULL, $5, false)); }
    ;

roles_block
    : {
        /* create a new roles block. */
        MAYBE_ASSIGN($$, new_roles(context)); }
    | roles_block IDENTIFIER LBRACE role_verbs RBRACE {
        MAYBE_ASSIGN($$, add_role(context, $1, $2, NULL, $4)); }
    | roles_block IDENTIFIER EXTENDS IDENTIFIER LBRACE role_verbs RBRACE {
        MAYBE_ASSIGN($$, add_role(context, $1, $2, $4, $6)); }
    ;

role_verbs
    : {
        /* create a new role_verbs block. */
        MAYBE_ASSIGN($$, new_role_verbs(context)); }
    | role_verbs IDENTIFIER {
        MAYBE_ASSIGN($$, add_role_verb(context, $1, $2)); }
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
    resource* dup;

    while (rbtree_count(entities) > 0)
    {
        node = rbtree_root_node(entities);
        value = (endorse_entity*)rbtree_node_value(entities, node);

        /* see if this entity already exists in the entities tree. */
        retval = rbtree_find(&dup, cfg->entities, value->id);
        if (STATUS_SUCCESS == retval)
        {
            char buffer[1024];
            snprintf(
                buffer, sizeof(buffer), "Duplicate entity `%s'.", value->id);
            CONFIG_ERROR(buffer);
        }

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
 * \brief Create a new role_verbs tree.
 */
static rbtree* new_role_verbs(endorse_config_context* context)
{
    status retval;
    rbtree* role_verbs;

    /* create a role_verbs rbtree. */
    retval =
        rbtree_create(
            &role_verbs, context->alloc, &endorse_role_verbs_compare,
            &endorse_role_verbs_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory creating role_verbs tree.");
    }

    return role_verbs;
}

/**
 * \brief Compare two role_verbs by key.
 */
static rcpr_comparison_result endorse_role_verbs_compare(
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
 * \brief Get the key for an endorse config role verb.
 */
static const void* endorse_role_verbs_key(
    void*, const resource* r)
{
    const endorse_role_verb* role = (const endorse_role_verb*)r;

    return (const void*)role->verb_name;
}

/**
 * \brief Create a new roles tree.
 */
static rbtree* new_roles(endorse_config_context* context)
{
    status retval;
    rbtree* roles;

    /* create a verbs rbtree. */
    retval =
        rbtree_create(
            &roles, context->alloc, &endorse_roles_compare,
            &endorse_roles_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory creating roles tree.");
    }

    return roles;
}

/**
 * \brief Compare two roles by key.
 */
static rcpr_comparison_result endorse_roles_compare(
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
 * \brief Get the key for an endorse config role.
 */
static const void* endorse_roles_key(
    void*, const resource* r)
{
    const endorse_role* role = (const endorse_role*)r;

    return (const void*)role->name;
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
    rbtree* roles = NULL;
    const char* error_message;

    /* create the verbs tree for this entity. */
    verbs = new_verbs(context);
    if (NULL == verbs)
    {
        error_message = "Out of memory creating entity verb tree in add_entity";
        goto error_exit;
    }

    /* create the roles tree for this entity. */
    roles = new_roles(context);
    if (NULL == roles)
    {
        error_message = "Out of memory creating roles tree in add_entity";
        goto cleanup_verbs;
    }

    /* create an entity. */
    entity = new_entity(context, id, verbs, roles, is_decl);
    if (NULL == entity)
    {
        error_message = "Out of memory creating entity in add_entity.";
        goto cleanup_roles;
    }

    /* verbs is now owned by entity. */
    verbs = NULL;

    /* roles is now owned by entity. */
    roles = NULL;

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

cleanup_roles:
    if (NULL != roles)
    {
        retval = resource_release(rbtree_resource_handle(roles));
        if (STATUS_SUCCESS != retval)
        {
            error_message = "Error releasing roles resource.";
        }
        roles = NULL;
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
    rbtree* roles, bool is_decl)
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
    entity->roles = roles;

    /* create verbs rbtree if NULL. */
    if (NULL == entity->verbs)
    {
        entity->verbs = new_verbs(context);
        if (NULL == entity->verbs)
        {
            error_message = "Out of memory creating verbs in new_entity.";
            goto cleanup_entity;
        }
    }

    /* create roles rbtree if NULL. */
    if (NULL == entity->roles)
    {
        entity->roles = new_roles(context);
        if (NULL == entity->roles)
        {
            error_message = "Out of memory creating roles in new_entity.";
            goto cleanup_entity;
        }
    }

    /* success. */
    return entity;

cleanup_entity:
    retval = resource_release(&entity->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Error releasing entity in new_entity.";
    }

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
    status roles_release_retval = STATUS_SUCCESS;
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
    if (NULL != entity->verbs)
    {
        verbs_release_retval =
            resource_release(rbtree_resource_handle(entity->verbs));
    }

    /* release the roles rbtree if set. */
    if (NULL != entity->roles)
    {
        roles_release_retval =
            resource_release(rbtree_resource_handle(entity->roles));
    }

    /* clear memory. */
    memset(entity, 0, sizeof(*entity));

    /* reclaim entity. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, entity);

    /* decode return status. */
    if (STATUS_SUCCESS != verbs_release_retval)
    {
        return verbs_release_retval;
    }
    else if (STATUS_SUCCESS != roles_release_retval)
    {
        return roles_release_retval;
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
    resource* dup;
    char buffer[1024];

    /* check to see if the verb already exists. */
    retval = rbtree_find(&dup, verbs, verb_name);
    if (STATUS_SUCCESS == retval)
    {
        snprintf(buffer, sizeof(buffer), "Duplicate verb `%s'.", verb_name);
        error_message = buffer;
        goto error_exit;
    }

    /* create a verb. */
    verb = new_verb(context, verb_name, verb_id);
    if (NULL == verb)
    {
        error_message = "Out of memory creating verb in add_verb.";
        goto error_exit;
    }

    /* insert this verb into the rbtree. */
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
        retval = merge_verbs(context, (endorse_entity*)entity_resource, entity);
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
    endorse_config_context* context, endorse_entity* canonical_entity,
    endorse_entity* entity)
{
    status retval;
    rbtree_node* node;
    endorse_verb* verb;
    resource* verb_resource;
    resource* dup;

    while (rbtree_count(entity->verbs) > 0)
    {
        node = rbtree_root_node(entity->verbs);
        verb = (endorse_verb*)rbtree_node_value(entity->verbs, node);

        /* see if this verb already exists in the verbs tree. */
        retval = rbtree_find(&dup, canonical_entity->verbs, verb->verb);
        if (STATUS_SUCCESS == retval)
        {
            char buffer[1024];
            snprintf(
                buffer, sizeof(buffer), "Duplicate verb `%s' for entity `%s'.",
                verb->verb, canonical_entity->id);
            context->set_error(context, buffer);
            return -1;
        }

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
 * \brief Merge an entity with roles with the canonical entity.
 */
static endorse_config* merge_role_entity(
    endorse_config_context* context, endorse_config* cfg,
    endorse_entity* entity)
{
    status retval;
    resource* entity_resource;

    /* attempt to look up the canonical entity by name. */
    retval = rbtree_find(&entity_resource, cfg->entities, entity->id);
    if (STATUS_SUCCESS == retval)
    {
        /* entity found; merge roles. */
        retval = merge_roles(context, (endorse_entity*)entity_resource, entity);
        if (STATUS_SUCCESS != retval)
        {
            CONFIG_ERROR("Error merging roles into the canonical entity.");
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
 * \brief Merge roles into the canonical entity.
 */
static status merge_roles(
    endorse_config_context* context, endorse_entity* canonical_entity,
    endorse_entity* entity)
{
    status retval;
    rbtree_node* node;
    endorse_role* role;
    resource* role_resource;
    resource* dup;

    /* less simple case: we need to merge the roles. */
    while (rbtree_count(entity->roles) > 0)
    {
        node = rbtree_root_node(entity->roles);
        role = (endorse_role*)rbtree_node_value(entity->roles, node);

        /* see if this role already exists in the verbs tree. */
        retval = rbtree_find(&dup, canonical_entity->roles, role->name);
        if (STATUS_SUCCESS == retval)
        {
            char buffer[1024];
            snprintf(
                buffer, sizeof(buffer), "Duplicate role `%s' for entity `%s'.",
                role->name, canonical_entity->id);
            context->set_error(context, buffer);
            return -1;
        }

        /* delete this node from the entity node tree. */
        retval = rbtree_delete(&role_resource, entity->roles, role->name);
        if (STATUS_SUCCESS != retval)
        {
            return retval;
        }

        /* insert this role into the canonical entity's role tree. */
        retval = rbtree_insert(canonical_entity->roles, role_resource);
        if (STATUS_SUCCESS != retval)
        {
            return retval;
        }
    }

    /* success. */
    return STATUS_SUCCESS;
}

/**
 * \brief Add a role to the roles tree.
 */
static rbtree* add_role(
    endorse_config_context* context, rbtree* roles, const char* role_name,
    const char* extends_role_name, rbtree* role_verbs)
{
    status retval;
    endorse_role* role = NULL;
    const char* error_message;
    resource* dup;
    char buffer[1024];

    /* check to see if the role already exists. */
    retval = rbtree_find(&dup, roles, role_name);
    if (STATUS_SUCCESS == retval)
    {
        snprintf(buffer, sizeof(buffer), "Duplicate role `%s'.", role_name);
        error_message = buffer;
        goto error_exit;
    }

    /* create a role. */
    role = new_role(context, role_name, extends_role_name, role_verbs);
    if (NULL == role)
    {
        error_message = "Out of memory creating role in add_role.";
        goto error_exit;
    }

    /* insert this role into the rbtree. */
    retval = rbtree_insert(roles, &role->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory inserting role in add_role.";
        goto cleanup_role;
    }

    /* success. */
    return roles;

cleanup_role:
    retval = resource_release(&role->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Error releasing role resource.";
    }

error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Create a new role.
 */
static endorse_role* new_role(
    endorse_config_context* context, const char* role_name,
    const char* extends_role_name, rbtree* role_verbs)
{
    status retval;
    endorse_role* role;
    const char* error_message;
    rbtree_node* node;
    endorse_role_verb* role_verb;
    char buffer[1024];
    resource* role_verb_resource;
    resource* dup;

    /* allocate an endorse_role instance. */
    retval =
        rcpr_allocator_allocate(
            context->alloc, (void**)&role, sizeof(*role));
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory creating role in new_role.";
        goto error_exit;
    }

    /* set up role. */
    memset(role, 0, sizeof(*role));
    resource_init(&role->hdr, &role_resource_release);
    role->alloc = context->alloc;
    role->name = strdup(role_name);
    role->reference_count = 1;

    /* if the extends role name is defined, duplicate it. */
    if (NULL != extends_role_name)
    {
        role->extends_role_name = strdup(extends_role_name);
    }

    /* create new role verbs tree. */
    role->verbs = new_role_verbs(context);
    if (NULL == role->verbs)
    {
        error_message = "Out of memory creating role verbs tree in new_role.";
        goto cleanup_role;
    }

    /* move verbs to tree. */
    while (rbtree_count(role_verbs) > 0)
    {
        node = rbtree_root_node(role_verbs);
        role_verb = (endorse_role_verb*)rbtree_node_value(role_verbs, node);

        /* see if this role_verb already exists in the role_verbs treee. */
        retval = rbtree_find(&dup, role->verbs, role_verb->verb_name);
        if (STATUS_SUCCESS == retval)
        {
            snprintf(
                buffer, sizeof(buffer), "Duplicate verb `%s' for role `%s'.",
                role_verb->verb_name, role->name);
            error_message = buffer;
            goto cleanup_role;
        }

        /* delete this node from the role verb tree. */
        retval =
            rbtree_delete(
                &role_verb_resource, role_verbs, role_verb->verb_name);
        if (STATUS_SUCCESS != retval)
        {
            error_message = "Error deleting role verb from role_verb tree.";
            goto cleanup_role;
        }

        /* insert this role verb into the role's role verb tree. */
        retval = rbtree_insert(role->verbs, role_verb_resource);
        if (STATUS_SUCCESS != retval)
        {
            error_message =
                "Error inserting role verb into role's role verb tree.";
            goto cleanup_role;
        }
    }

    /* success. */
    return role;

cleanup_role:
    retval = resource_release(&role->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Error releasing role in new_role.";
    }
    
error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Release a role resource.
 */
static status role_resource_release(resource* r)
{
    endorse_role* role = (endorse_role*)r;
    status role_verbs_release_retval = STATUS_SUCCESS;
    status role_extends_release_retval = STATUS_SUCCESS;
    status role_reclaim_retval = STATUS_SUCCESS;

    /* decrement reference count. */
    --role->reference_count;

    /* if there are still references, don't release this resource. */
    if (role->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache allocator. */
    rcpr_allocator* alloc = role->alloc;

    /* free the role name string. */
    free((void*)role->name);

    /* if the extends role name is set, free it. */
    if (NULL != role->extends_role_name)
    {
        free((void*)role->extends_role_name);
    }

    /* release the role verbs tree if set. */
    if (NULL != role->verbs)
    {
        role_verbs_release_retval =
            resource_release(rbtree_resource_handle(role->verbs));
    }

    /* release the extended role if set. */
    if (NULL != role->extends_role)
    {
        role_extends_release_retval =
            resource_release(&role->extends_role->hdr);
    }

    /* clear memory. */
    memset(role, 0, sizeof(*role));

    /* reclaim role. */
    role_reclaim_retval = rcpr_allocator_reclaim(alloc, role);

    /* decode response code. */
    if (STATUS_SUCCESS != role_verbs_release_retval)
    {
        return role_verbs_release_retval;
    }
    else if (STATUS_SUCCESS != role_extends_release_retval)
    {
        return role_extends_release_retval;
    }
    else
    {
        return role_reclaim_retval;
    }
}

/**
 * \brief Add the given role verb to the role verbs tree.
 */
static rbtree* add_role_verb(
    endorse_config_context* context, rbtree* role_verbs, const char* verb_name)
{
    status retval;
    endorse_role_verb* role_verb = NULL;
    const char* error_message;
    resource* dup;
    char buffer[1024];

    /* check to see if the role verb already exists. */
    retval = rbtree_find(&dup, role_verbs, verb_name);
    if (STATUS_SUCCESS == retval)
    {
        snprintf(
            buffer, sizeof(buffer), "Duplicate verb `%s' for role.", verb_name);
        error_message = buffer;
        goto error_exit;
    }

    /* create a role_verb. */
    role_verb = new_role_verb(context, verb_name, NULL);
    if (NULL == role_verb)
    {
        error_message = "Out of memory creating role in add_role_verb.";
        goto error_exit;
    }

    /* insert this role verb into the rbtree. */
    retval = rbtree_insert(role_verbs, &role_verb->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory inserting role_verb in add_role_verb.";
        goto cleanup_role_verb;
    }

    /* success. */
    return role_verbs;

cleanup_role_verb:
    retval = resource_release(&role_verb->hdr);
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Error releasing role_verb resource.";
    }

error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Create a new role verb.
 */
static endorse_role_verb* new_role_verb(
    endorse_config_context* context, const char* verb_name, endorse_verb* verb)
{
    status retval;
    endorse_role_verb* role_verb;
    const char* error_message;

    /* allocate an endorse_role_verb instance. */
    retval =
        rcpr_allocator_allocate(
            context->alloc, (void**)&role_verb, sizeof(*role_verb));
    if (STATUS_SUCCESS != retval)
    {
        error_message = "Out of memory creating role verb in new_role_verb.";
        goto error_exit;
    }

    /* set up role_verb. */
    memset(role_verb, 0, sizeof(*role_verb));
    resource_init(&role_verb->hdr, &role_verb_resource_release);
    role_verb->alloc = context->alloc;
    role_verb->verb_name = strdup(verb_name);
    role_verb->verb = verb;
    role_verb->reference_count = 1;

    /* increment reference count on role_verb->verb if set. */
    if (NULL != role_verb->verb)
    {
        ++role_verb->verb->reference_count;
    }

    /* success. */
    return role_verb;

error_exit:
    CONFIG_ERROR(error_message);
}

/**
 * \brief Release a role verb resource.
 */
static status role_verb_resource_release(resource* r)
{
    endorse_role_verb* role_verb = (endorse_role_verb*)r;
    status role_verb_verb_release_retval = STATUS_SUCCESS;
    status role_verb_reclaim_retval = STATUS_SUCCESS;

    /* decrement reference count. */
    --role_verb->reference_count;

    /* if there are still references, don't release this resource. */
    if (role_verb->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache allocator. */
    rcpr_allocator* alloc = role_verb->alloc;

    /* free the verb name string. */
    free((void*)role_verb->verb_name);

    /* release the verb resource if set. */
    if (NULL != role_verb->verb)
    {
        role_verb_verb_release_retval = resource_release(&role_verb->verb->hdr);
    }

    /* clear memory. */
    memset(role_verb, 0, sizeof(*role_verb));

    /* reclaim role_verb. */
    role_verb_reclaim_retval = rcpr_allocator_reclaim(alloc, role_verb);

    /* decode response code. */
    if (STATUS_SUCCESS != role_verb_verb_release_retval)
    {
        return role_verb_verb_release_retval;
    }
    else
    {
        return role_verb_reclaim_retval;
    }
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

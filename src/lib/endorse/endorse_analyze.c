/**
 * \file endorse/endorse_analyze.c
 *
 * \brief Analyze the AST from an endorse config file.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <stdio.h>
#include <vctool/endorse.h>

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/* forward decls. */
static status endorse_analyze_entity_roles(
    endorse_config_context* context, endorse_entity* entity);
static status endorse_analyze_entity_role_verbs(
    endorse_config_context* context, endorse_entity* entity,
    endorse_role* role);

/**
 * \brief Analyze the AST produced by the endorse file parser and finish
 * populating the AST with relevant data.
 *
 * \param context       The endorse config context for this parse.
 * \param root          The AST root to analyze.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_analyze(endorse_config_context* context, endorse_config* root)
{
    rbtree_node* node = NULL;
    rbtree_node* nil = NULL;
    char buffer[1024];
    bool fail = false;
    status retval = STATUS_SUCCESS;

    /* get the nil node. */
    nil = rbtree_nil_node(root->entities);

    /* get the root node. */
    node = rbtree_root_node(root->entities);
    if (nil == node)
    {
        /* empty config. */
        return STATUS_SUCCESS;
    }

    /* iterate through all entities. */
    for (
        node = rbtree_minimum_node(root->entities, node);
        node != nil;
        node = rbtree_successor_node(root->entities, node))
    {
        /* get the value of this node. */
        endorse_entity* entity =
            (endorse_entity*)rbtree_node_value(root->entities, node);

        /* has this entity been declared? */
        if (!entity->id_declared)
        {
            snprintf(
                buffer, sizeof(buffer),
                "Entity `%s' not declared before being used.\n", entity->id);
            context->set_error(context, buffer);
            fail = true;
        }

        /* iterate through the roles. */
        retval = endorse_analyze_entity_roles(context, entity);
        if (STATUS_SUCCESS != retval)
        {
            fail = true;
        }
    }

    /* if we encountered a failure, return a non-zero error code. */
    if (fail)
    {
        return -1;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

/**
 * \brief Analyze all defined roles for a given entity.
 *
 * \param context       The endorse config context for this operation.
 * \param entity        The entity to analyze.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status endorse_analyze_entity_roles(
    endorse_config_context* context, endorse_entity* entity)
{
    status retval = STATUS_SUCCESS;
    rbtree_node* node = NULL;
    rbtree_node* nil = NULL;
    bool fail = false;

    /* get the nil node. */
    nil = rbtree_nil_node(entity->roles);

    /* get the root node. */
    node = rbtree_root_node(entity->roles);
    if (nil == node)
    {
        /* no roles. */
        return STATUS_SUCCESS;
    }

    /* iterate through all roles. */
    for (
        node = rbtree_minimum_node(entity->roles, node);
        node != nil;
        node = rbtree_successor_node(entity->roles, node))
    {
        /* get the value of this node. */
        endorse_role* role =
            (endorse_role*)rbtree_node_value(entity->roles, node);

        /* iterate through the verbs. */
        retval = endorse_analyze_entity_role_verbs(context, entity, role);
        if (STATUS_SUCCESS != retval)
        {
            fail = true;
        }
    }

    /* if we encountered a failure, return a non-zero error code. */
    if (fail)
    {
        return -1;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

/**
 * \brief Analyze all declared verbs for a given role.
 *
 * \param context       The endorse config context for this operation.
 * \param entity        The entity to analyze.
 * \param role          The role to analyze.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status endorse_analyze_entity_role_verbs(
    endorse_config_context* context, endorse_entity* entity,
    endorse_role* role)
{
    status retval = STATUS_SUCCESS;
    rbtree_node* node = NULL;
    rbtree_node* nil = NULL;
    resource* verb_resource = NULL;
    bool fail = false;
    char buffer[1024];

    /* get the nil node. */
    nil = rbtree_nil_node(role->verbs);

    /* get the root node. */
    node = rbtree_root_node(role->verbs);
    if (nil == node)
    {
        /* no verbs. */
        return STATUS_SUCCESS;
    }

    /* iterate through all verbs. */
    for (
        node = rbtree_minimum_node(role->verbs, node);
        node != nil;
        node = rbtree_successor_node(role->verbs, node))
    {
        /* get the value of this node. */
        endorse_role_verb* role_verb =
            (endorse_role_verb*)rbtree_node_value(role->verbs, node);

        /* attempt to look up this verb in the entity. */
        retval =
            rbtree_find(&verb_resource, entity->verbs, role_verb->verb_name);
        if (STATUS_SUCCESS != retval)
        {
            snprintf(
                buffer, sizeof(buffer),
                "Entity `%s' role `%s' references undefined verb `%s'.\n",
                entity->id, role->name, role_verb->verb_name);
            context->set_error(context, buffer);
            fail = true;
        }
        else
        {
            role_verb->verb = (endorse_verb*)verb_resource;
            ++(role_verb->verb->reference_count);
        }
    }

    /* did semantic analysis fail? */
    if (fail)
    {
        return -1;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

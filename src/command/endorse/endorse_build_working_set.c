/**
 * \file command/endorse/endorse_build_working_set.c
 *
 * \brief Build a working set of capabilities from the uuid dictionary, ast, and
 * command-line.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/**
 * \brief Build a working set of capabilities using the AST and uuid dictionary.
 *
 * \param set               Receive a pointer to the working set on success.
 * \param alloc             The allocator to use for this operation.
 * \param root              The root command config.
 * \param ast               The ast to use for this operation.
 * \param dict              The uuid dictionary to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_build_working_set(
    RCPR_SYM(rbtree)** set, RCPR_SYM(allocator)* alloc,
    const root_command* root, const endorse_config* ast,
    RCPR_SYM(rbtree)* dict)
{
    status retval, release_retval;
    rbtree* tmp;
    slist_node* x;
    root_permission* perm;
    endorse_uuid_dictionary_entry* uuid_entry;
    endorse_entity* entity;

    /* attempt to create an rbtree instance. */
    retval =
        rbtree_create(
            &tmp, alloc, &endorse_working_set_compare,
            &endorse_working_set_get_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* get the first node for the permission list. */
    retval = slist_head(&x, root->permissions);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_working_set;
    }

    /* are there any entries? */
    if (NULL == x)
    {
        fprintf(stderr, "No permissions specified.\n");
    }

    /* iterate through all nodes. */
    while (NULL != x)
    {
        /* get the child. */
        retval = slist_node_child((resource**)&perm, x);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_working_set;
        }

        /* attempt to look up the entity in the uuid dictionary. */
        retval = rbtree_find((resource**)&uuid_entry, dict, perm->entity);
        if (STATUS_SUCCESS != retval)
        {
            fprintf(stderr, "UUID for %s was not specified.\n", perm->entity);
            goto cleanup_working_set;
        }

        /* attempt to look up the entity in the ast. */
        retval = rbtree_find((resource**)&entity, ast->entities, perm->entity);
        if (STATUS_SUCCESS != retval)
        {
            fprintf(
                stderr, "Entity %s is not defined in endorse config.\n",
                perm->entity);
            goto cleanup_working_set;
        }

        /* Given the entity, entity UUID, and moiety: */
        /* populate the working set with all capability UUIDs. */
        retval =
            endorse_working_set_add_capabilities(
                tmp, alloc, entity, &uuid_entry->value, perm->moiety);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_working_set;
        }

        /* get the next node. */
        retval = slist_node_next(&x, x);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_working_set;
        }
    }

    /* success. The caller now owns the working set. */
    *set = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_working_set:
    release_retval = resource_release(rbtree_resource_handle(tmp));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

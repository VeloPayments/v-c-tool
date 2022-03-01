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

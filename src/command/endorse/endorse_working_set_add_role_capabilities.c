/**
 * \file command/endorse/endorse_working_set_add_role_capabilities.c
 *
 * \brief Add all of the capabilities for the given role to the working set.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_rbtree;

/**
 * \brief Add all of the capabilities for the given role to the working set.
 *
 * \param set               The current working set.
 * \param alloc             The allocator to use for this operation.
 * \param entity_id         The ID of this entity.
 * \param role              The role to add to this working set.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_working_set_add_role_capabilities(
    RCPR_SYM(rbtree)* set, RCPR_SYM(allocator)* alloc,
    const RCPR_SYM(rcpr_uuid)* entity_id, endorse_role* role)
{
    status retval;
    rbtree_node* nil;
    rbtree_node* tmp;
    endorse_role_verb* role_verb;

    /* get the nil node of the role verbs tree. */
    nil = rbtree_nil_node(role->verbs);

    /* get the root node of the role verbs tree. */
    tmp = rbtree_root_node(role->verbs);
    if (nil == tmp)
    {
        /* no verbs to add. */
        retval = STATUS_SUCCESS;
        goto done;
    }

    /* get the minimum node of this subtree. */
    tmp = rbtree_minimum_node(role->verbs, tmp);

    /* loop through all verbs. */
    while (nil != tmp)
    {
        /* get the role verb for this entry. */
        role_verb = (endorse_role_verb*)rbtree_node_value(role->verbs, tmp);

        /* add the verb capability to the working set. */
        retval =
            endorse_working_set_add_verb_capability(
                set, alloc, entity_id, role_verb->verb);
        if (STATUS_SUCCESS != retval)
        {
            goto done;
        }

        /* get the next node in the subtree. */
        tmp = rbtree_successor_node(role->verbs, tmp);
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

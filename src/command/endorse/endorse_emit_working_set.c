/**
 * \file command/endorse/endorse_emit_working_set.c
 *
 * \brief Emit the working set to the builder.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <vccert/fields.h>

#include "endorse_internal.h"

RCPR_IMPORT_rbtree;

/**
 * \brief Write the working set to the builder.
 *
 * \param builder           The builder to which the fields are written.
 * \param pub_id            The uuid of the entity being granted endorsements.
 * \param set               The working set to write to the builder.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_emit_working_set(
    vccert_builder_context_t* builder, const RCPR_SYM(rcpr_uuid)* pub_id,
    RCPR_SYM(rbtree)* set)
{
    status retval;
    rbtree_node* nil;
    rbtree_node* x;
    endorse_working_set_entry* entry;
    uint8_t endorsement_data[3 * 16];

    /* get the nil node for the working set. */
    nil = rbtree_nil_node(set);

    /* start x at the root node for the working set. */
    x = rbtree_root_node(set);
    if (nil == x)
    {
        /* no work to do. */
        retval = STATUS_SUCCESS;
        goto done;
    }

    /* move to the minimum node of this tree. */
    x = rbtree_minimum_node(set, x);

    /* iterate through the tree. */
    while (nil != x)
    {
        /* get the child of this node. */
        entry = (endorse_working_set_entry*)rbtree_node_value(set, x);

        /* clear the endorsement data. */
        memset(endorsement_data, 0, sizeof(endorsement_data));

        /* write the subject at the start of the endorsement data. */
        memcpy(endorsement_data, pub_id, sizeof(*pub_id));

        /* write the verb in the middle. */
        memcpy(
            endorsement_data + 16, &entry->key.verb, sizeof(entry->key.verb));

        /* write the object at the end. */
        memcpy(
            endorsement_data + 32, &entry->key.object,
            sizeof(entry->key.object));

        /* write this data to the builder. */
        retval =
            vccert_builder_add_short_buffer(
                builder, VCCERT_FIELD_TYPE_VELO_ENDORSEMENT, endorsement_data,
                sizeof(endorsement_data));
        if (STATUS_SUCCESS != retval)
        {
            goto done;
        }

        /* move to the next node in the tree. */
        x = rbtree_successor_node(set, x);
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

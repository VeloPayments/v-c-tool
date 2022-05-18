/**
 * \file command/endorse/endorse_working_set_add_verb_capability.c
 *
 * \brief Add the capability associated with the given verb to the working set.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Add the capability associated with the given verb to the working set.
 *
 * \param set               The current working set.
 * \param alloc             The allocator to use for this operation.
 * \param entity_id         The ID of this entity.
 * \param verb              The verb to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_working_set_add_verb_capability(
    RCPR_SYM(rbtree)* set, RCPR_SYM(allocator)* alloc,
    const RCPR_SYM(rcpr_uuid)* entity_id, endorse_verb* verb)
{
    status retval, release_retval;
    endorse_working_set_key key;
    endorse_working_set_entry* tmp;

    /* create the key for the working set capability. */
    memset(&key, 0, sizeof(key));
    memcpy(&key.object, entity_id, sizeof(key.object));
    memcpy(&key.verb, &verb->verb_id, sizeof(key.verb));

    /* first, check to see if this working set capability exists. */
    retval = rbtree_find((resource**)&tmp, set, &key);
    if (STATUS_SUCCESS == retval)
    {
        /* the capability already exists; we can exit. */
        goto done;
    }

    /* it does not, so allocate memory for this entry. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(&tmp->hdr, &endorse_working_set_entry_resource_release);

    /* cache allocator. */
    tmp->alloc = alloc;

    /* copy key. */
    memcpy(&tmp->key, &key, sizeof(key));

    /* insert entry. */
    retval = rbtree_insert(set, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

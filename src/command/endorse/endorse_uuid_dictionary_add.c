/**
 * \file command/endorse/endorse_uuid_dictionary_add.c
 *
 * \brief Add an entry to the endorse uuid dictionary.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Add an entry to the uuid dictionary.
 *
 * \param dict              The dictionary to which this entry is added.
 * \param alloc             The allocator to use for this operation.
 * \param key               The key for this entry.
 * \param value             The UUID value for this entry.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_uuid_dictionary_add(
    RCPR_SYM(rbtree)* dict, RCPR_SYM(allocator)* alloc, const char* key,
    const RCPR_SYM(rcpr_uuid)* value)
{
    status retval, release_retval;
    endorse_uuid_dictionary_entry* tmp;

    /* allocate memory for this entry. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(&tmp->hdr, &endorse_uuid_dictionary_entry_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    tmp->key = strdup(key);
    memcpy(&tmp->value, value, sizeof(tmp->value));

    /* attempt to insert this entry into the dictionary. */
    retval = rbtree_insert(dict, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_entry;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_entry:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

/**
 * \file command/endorse/endorse_uuid_dictionary_entry_resource_release.c
 *
 * \brief Release an endorse uuid dictionary entry resource.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release an endorse uuid dictionary entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_uuid_dictionary_entry_resource_release(RCPR_SYM(resource)* r)
{
    endorse_uuid_dictionary_entry* entry = (endorse_uuid_dictionary_entry*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* free key if set. */
    if (NULL != entry->key)
    {
        free(entry->key);
    }

    /* reclaim memory for this entry. */
    return
        rcpr_allocator_reclaim(alloc, entry);
}

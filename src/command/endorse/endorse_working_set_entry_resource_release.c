/**
 * \file command/endorse/endorse_working_set_entry_resource_release.c
 *
 * \brief Release an endorse working set entry resource.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release an endorse working set entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_working_set_entry_resource_release(RCPR_SYM(resource)* r)
{
    endorse_working_set_entry* entry = (endorse_working_set_entry*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, entry);
}

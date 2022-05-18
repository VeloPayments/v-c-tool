/**
 * \file command/endorse/endorse_working_set_get_key.c
 *
 * \brief Get the key for a working set entry.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

/**
 * \brief Given an endorse_working_set_entry, return the key.
 *
 * \param context       Unused.
 * \param r             The resource handle of the
 *                      endorse_working_set_entry.
 *
 * \returns the key for the entry.
 */
const void* endorse_working_set_get_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    endorse_working_set_entry* entry = (endorse_working_set_entry*)r;

    return &entry->key;
}

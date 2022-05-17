/**
 * \file command/endorse/endorse_uuid_dictionary_key.c
 *
 * \brief Get the key value for an endorse_uuid_dictionary_entry.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

/**
 * \brief Given an endorse_uuid_dictionary_entry, return the key.
 *
 * \param context       Unused.
 * \param r             The resource handle of the
 *                      endorse_uuid_dictionary_entry.
 *
 * \returns the key for the entry.
 */
const void* endorse_uuid_dictionary_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    endorse_uuid_dictionary_entry* entry = (endorse_uuid_dictionary_entry*)r;

    return entry->key;
}

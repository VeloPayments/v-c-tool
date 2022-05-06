/**
 * \file lib/endorse/endorse_config_error_message_resource_release.c
 *
 * \brief Release an error message node.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release an error message node.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_error_message_node_resource_release(
    RCPR_SYM(resource)* r)
{
    endorse_config_error_message_node* node =
        (endorse_config_error_message_node*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = node->alloc;

    /* clean up error message, if set. */
    if (NULL != node->msg)
    {
        free(node->msg);
    }

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, node);
}

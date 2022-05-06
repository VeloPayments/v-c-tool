/**
 * \file lib/endorse/endorse_config_default_user_context_resource_release.c
 *
 * \brief Release the default user context for the endorse parser.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/**
 * \brief Release a default config resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_default_user_context_resource_release(
    RCPR_SYM(resource)* r)
{
    status slist_release_retval = STATUS_SUCCESS;
    status config_release_retval = STATUS_SUCCESS;
    status reclaim_retval = STATUS_SUCCESS;
    endorse_config_default_user_context* context =
        (endorse_config_default_user_context*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = context->alloc;

    /* release slist if initialized. */
    if (NULL != context->error_list)
    {
        slist_release_retval =
            resource_release(slist_resource_handle(context->error_list));
    }

    /* release AST if initialized. */
    if (NULL != context->root)
    {
        config_release_retval =
            resource_release(&context->root->hdr);
    }

    /* reclaim memory. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, context);

    /* decode return value. */
    if (STATUS_SUCCESS != slist_release_retval)
    {
        return slist_release_retval;
    }
    else if (STATUS_SUCCESS != config_release_retval)
    {
        return config_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}

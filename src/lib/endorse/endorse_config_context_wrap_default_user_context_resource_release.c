/**
 * \file
 * lib/endorse/endorse_config_context_wrap_default_user_context_resource_release.c
 *
 * \brief Resource release wrapper to free default context when the endorse
 * context is released.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_resource;

/**
 * \brief Wrap the endorse context resource release function to release our
 * default config context.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_context_wrap_default_user_context_resource_release(
    RCPR_SYM(resource)* r)
{
    status release_user_config_retval = STATUS_SUCCESS;
    status release_retval = STATUS_SUCCESS;
    endorse_config_context* context = (endorse_config_context*)r;
    endorse_config_default_user_context* user =
        (endorse_config_default_user_context*)context->user_context;

    /* restore the original resource release method. */
    context->hdr.release = user->parent_old_hdr.release;

    /* release the user context. */
    release_user_config_retval = resource_release(&user->hdr);

    /* release the context. */
    release_retval = resource_release(&context->hdr);

    /* decode return value. */
    if (STATUS_SUCCESS != release_user_config_retval)
    {
        return release_user_config_retval;
    }
    else
    {
        return release_retval;
    }
}

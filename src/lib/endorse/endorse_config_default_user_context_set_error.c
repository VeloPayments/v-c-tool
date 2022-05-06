/**
 * \file lib/endorse/endorse_config_default_user_context_set_error.c
 *
 * \brief Set an error condition from the parse or the semantic analysis.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/**
 * \brief Set an error message in the default config.
 *
 * \param context       The endorse config context for this operation.
 * \param error         The error message for this operation.
 */
void endorse_config_default_user_context_set_error(
    endorse_config_context* context, const char* error)
{
    status retval, release_retval;
    endorse_config_error_message_node* tmp;
    endorse_config_default_user_context* user =
        (endorse_config_default_user_context*)context->user_context;

    /* create an error message node. */
    retval =
        endorse_config_error_message_node_create(&tmp, context->alloc, error);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* append this node to the user context error list. */
    retval = slist_append_tail(user->error_list, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_node;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_node:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
}

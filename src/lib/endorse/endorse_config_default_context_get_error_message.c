/**
 * \file lib/endorse/endorse_config_default_context_get_error_message.c
 *
 * \brief Get the Nth error message from the default endorse config.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <vctool/status_codes.h>

#include "endorse_internal.h"

RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/**
 * \brief Get the Nth error message from the default endorse config.
 *
 * \param msg           Pointer to receive the error message pointer. This error
 *                      message is owned by the config context and must not be
 *                      freed.
 * \param context       Pointer to receive the default context.
 * \param index         The error message index.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_default_context_get_error_message(
    const char** msg, const endorse_config_context* context, int index)
{
    status retval;
    slist_node* node;
    resource* r;
    endorse_config_error_message_node* msgnode;
    endorse_config_default_user_context* user =
        (endorse_config_default_user_context*)context->user_context;

    /* get the first slist node. */
    retval = slist_head(&node, user->error_list);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* iterate through each error in the list. */
    while (index-- && NULL != node)
    {
        retval = slist_node_next(&node, node);
    }

    /* was the Nth entry found? */
    if (0 == index)
    {
        /* get the error message node. */
        retval = slist_node_child(&r, node);

        /* save the error message. */
        msgnode = (endorse_config_error_message_node*)r;
        *msg = msgnode->msg;

        /* success. */
        retval = STATUS_SUCCESS;
        goto done;
    }

    /* the index is out of bounds. Return an error. */
    retval = VCTOOL_ERROR_ENDORSE_ERROR_MESSAGE_INDEX_OUT_OF_BOUNDS;
    goto done;

done:
    return retval;
}

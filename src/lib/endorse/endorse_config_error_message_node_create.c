/**
 * \file lib/endorse/endorse_config_error_message_node_create.c
 *
 * \brief Create an error message node.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create an error message node, duplicating an error string.
 *
 * \param node          Pointer to receive the new node.
 * \param alloc         The allocator to use.
 * \param msg           The error message string to duplicate.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_error_message_node_create(
    endorse_config_error_message_node** node, RCPR_SYM(allocator)* alloc,
    const char* msg)
{
    status retval, release_retval;
    endorse_config_error_message_node* tmp;

    /* allocate memory for this node. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear structure. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr, &endorse_config_error_message_node_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    tmp->msg = strdup(msg);

    /* verify that the message was copied. */
    if (NULL == tmp->msg)
    {
        retval = ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_tmp;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    *node = tmp;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

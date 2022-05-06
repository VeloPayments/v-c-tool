/**
 * \file lib/endorse/endorse_config_create_default.c
 *
 * \brief Create an endorse config context with a default user context.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/**
 * \brief Create a default endorse config context that saves a list of errors
 * and saves the endorse config root.
 *
 * \param context       Pointer to receive the default context.
 * \param alloc         The allocator to use to create this context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_create_default(
    endorse_config_context** context, RCPR_SYM(allocator)* alloc)
{
    status retval, release_retval;
    endorse_config_default_user_context* tmp;

    /* allocate memory for the default user context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr, &endorse_config_default_user_context_resource_release);

    /* set values. */
    tmp->alloc = alloc;

    /* initialize slist. */
    retval = slist_create(&tmp->error_list, alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* initialize context. */
    retval =
        endorse_config_create_ex(
            context, alloc, &endorse_config_default_user_context_set_error,
            &endorse_config_default_user_context_val_callback, tmp);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* save the original resource release method. */
    tmp->parent_old_hdr.release = (*context)->hdr.release;

    /* hook in the wrapper resource release method. */
    (*context)->hdr.release =
        &endorse_config_context_wrap_default_user_context_resource_release;

    /* success. */
    retval = STATUS_SUCCESS;
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

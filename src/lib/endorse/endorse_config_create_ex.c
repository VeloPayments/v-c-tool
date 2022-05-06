/**
 * \file lib/endorse/endorse_config_create_ex.c
 *
 * \brief Create an endorse config context for parsing.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/* forward decls. */
static status endorse_config_context_resource_release(resource* r);

/**
 * \brief Create an endorse context using the given set_error, value callback,
 * and user context values.
 *
 * \note This function creates a resource, but does not add resource management
 * for the user context. This is up to the caller.
 *
 * \param context       Pointer to receive the new context.
 * \param alloc         The allocator to use for this operation.
 * \param set_error     The set_error function callback.
 * \param val_callback  The value callback.
 * \param user_context  The user context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_create_ex(
    endorse_config_context** context, RCPR_SYM(allocator)* alloc,
    endorse_config_set_error_fn set_error,
    endorse_config_val_callback_fn val_callback, void* user_context)
{
    status retval;
    endorse_config_context* tmp;

    /* allocate memory for the context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear structure. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(&tmp->hdr, &endorse_config_context_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    tmp->set_error = set_error;
    tmp->val_callback = val_callback;
    tmp->user_context = user_context;

    /* Set the context on success. */
    *context = tmp;
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

/**
 * \brief Clean up context resource.
 *
 * \param r         The context resource to clean up.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status endorse_config_context_resource_release(resource* r)
{
    endorse_config_context* context = (endorse_config_context*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = context->alloc;

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, context);
}

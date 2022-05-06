/**
 * \file lib/endorse/endorse_internal.h
 *
 * \brief Internal declarations and definitions for endorse parser.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#pragma once

#include <rcpr/resource/protected.h>
#include <rcpr/slist.h>
#include <vctool/endorse.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The default endorse config user context defines a list of error
 * messages and storage for the config.
 */
typedef struct endorse_config_default_user_context
endorse_config_default_user_context;

struct endorse_config_default_user_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(resource) parent_old_hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(slist)* error_list;
    endorse_config* root;
};

/**
 * \brief Set an error message in the default config.
 *
 * \param context       The endorse config context for this operation.
 * \param error         The error message for this operation.
 */
void endorse_config_default_user_context_set_error(
    endorse_config_context* context, const char* error);

/**
 * \brief Set the config value in the default config.
 *
 * \param context       The endorse config context for this operation.
 * \param root          The root config value to set.
 */
void endorse_config_default_user_context_val_callback(
    endorse_config_context* context, endorse_config* root);

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
    RCPR_SYM(resource)* r);

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
    RCPR_SYM(resource)* r);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

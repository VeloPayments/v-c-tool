/**
 * \file lib/endorse/endorse_config_default_context_get_endorse_config_root.c
 *
 * \brief Get the root of the endorse config AST.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

/**
 * \brief Get the endorse config root.
 *
 * \param context       Pointer to the config context.
 *
 * \returns The endorse config root.
 */
const endorse_config* endorse_config_default_context_get_endorse_config_root(
    const endorse_config_context* context)
{
    endorse_config_default_user_context* user =
        (endorse_config_default_user_context*)context->user_context;

    return user->root;
}

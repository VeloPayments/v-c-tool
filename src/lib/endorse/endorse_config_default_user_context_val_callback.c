/**
 * \file lib/endorse/endorse_config_default_user_context_val_callback.c
 *
 * \brief Set the value from the parser.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

/**
 * \brief Set the config value in the default config.
 *
 * \param context       The endorse config context for this operation.
 * \param root          The root config value to set.
 */
void endorse_config_default_user_context_val_callback(
    endorse_config_context* context, endorse_config* root)
{
    endorse_config_default_user_context* user =
        (endorse_config_default_user_context*)context->user_context;

    /* set the config value. */
    user->root = root;
}

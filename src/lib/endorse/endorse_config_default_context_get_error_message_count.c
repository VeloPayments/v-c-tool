/**
 * \file lib/endorse/endorse_config_default_context_get_error_message_count.c
 *
 * \brief Get the number of error messages in the default context.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "endorse_internal.h"

RCPR_IMPORT_slist;

/**
 * \brief Get the number of error messages from the default endorse config.
 *
 * \param context       Pointer to the config context.
 *
 * \returns The number of error messages.
 */
size_t endorse_config_default_context_get_error_message_count(
    const endorse_config_context* context)
{
    endorse_config_default_user_context* user =
        (endorse_config_default_user_context*)context->user_context;

    return
        slist_count(user->error_list);
}

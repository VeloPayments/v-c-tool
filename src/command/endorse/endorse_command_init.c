/**
 * \file command/endorse/endorse_command_init.c
 *
 * \brief Initialize a endorse command structure.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/command/endorse.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>
#include <vpr/parameters.h>

/* forward decls. */
static void endorse_command_dispose(void* disp);

/**
 * \brief Initialize a endorse command structure.
 *
 * \param endorse       The endorse command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int endorse_command_init(endorse_command* endorse)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != endorse);

    /* clear endorse command structure. */
    memset(endorse, 0, sizeof(endorse_command));

    /* set disposer, func, etc. */
    endorse->hdr.hdr.dispose = &endorse_command_dispose;
    endorse->hdr.func = &endorse_command_func;

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a endorse_command structure.
 *
 * \param disp          The endorse_command structure to dispose.
 */
static void endorse_command_dispose(void* UNUSED(disp))
{
    /* do nothing. */
}

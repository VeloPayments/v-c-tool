/**
 * \file command/endorse/endorse_command_func.c
 *
 * \brief Entry point for the endorse command.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <vccrypt/compare.h>
#include <vctool/certificate.h>
#include <vctool/commandline.h>
#include <vctool/command/endorse.h>
#include <vctool/command/root.h>
#include <vctool/readpassword.h>

/**
 * \brief Execute the endorse command.
 *
 * \param opts          The commandline opts for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int endorse_command_func(commandline_opts* opts)
{
    (void)opts;

    fprintf(stderr, "endorse not yet implemented.\n");

    return VCTOOL_STATUS_SUCCESS;
}

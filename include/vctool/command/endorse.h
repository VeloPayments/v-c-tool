/**
 * \file include/vctool/command/keygen.h
 *
 * \brief Endorse command structure.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct endorse_command
{
    command hdr;
} endorse_command;

/**
 * \brief Initialize a endorse command structure.
 *
 * \param endorse       The endorse command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int endorse_command_init(endorse_command* endorse);

/**
 * \brief Process the endorse command.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int process_endorse_command(commandline_opts* opts, int argc, char* argv[]);

/**
 * \brief Execute the endorse command.
 *
 * \param opts          The commandline opts for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int endorse_command_func(commandline_opts* opts);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

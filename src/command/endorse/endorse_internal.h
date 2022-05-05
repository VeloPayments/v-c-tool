/**
 * \file command/endorse/endorse_internal.h
 *
 * \brief Internal header for the endorse command.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#pragma once

#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <vccrypt/compare.h>
#include <vctool/certificate.h>
#include <vctool/command/endorse.h>
#include <vctool/command/root.h>
#include <vctool/control.h>
#include <vctool/readpassword.h>

#include "certfile.h"

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the key certfile and output an error message if the key file
 * option is not set on the command line.
 *
 * \param key_file      Pointer to receive the key file certfile instance.
 * \param opts          The command-line options to use.
 * \param alloc         The allocator to use.
 * \param root          The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_key_file(
    certfile** key_file, commandline_opts* opts, RCPR_SYM(allocator)* alloc,
    const root_command* root);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

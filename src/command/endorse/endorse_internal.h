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

/**
 * \brief Get the input file and output an error message if the input file
 * option is not set on the command line.
 *
 * \param input_file    Pointer to receive the input file certfile instance.
 * \param opts          The command-line options to use.
 * \param alloc         The allocator to use.
 * \param root          The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_input_file(
    certfile** input_file, commandline_opts* opts, RCPR_SYM(allocator)* alloc,
    const root_command* root);

/**
 * \brief Get the output filename and output an error message if the output file
 * would clobber an existing file.
 *
 * \param output_filename    Pointer to receive the output filename.
 * \param opts               The command-line options to use.
 * \param input_filename     The input filename to use as a template if an
 *                           output filename is not specified on the
 *                           command-line.
 * \param root               The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_output_filename(
    char** output_filename, commandline_opts* opts, const char* input_filename,
    const root_command* root);

/**
 * \brief Read and optionally prompt for a passphrase and decrypt the key
 * certificate file.
 *
 * \param cert               Pointer to buffer that will be initialized with the
 *                           certificate.
 * \param opts               The command-line options to use.
 * \param key_file           The certfile from which the key certificate is
 *                           read.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_read_key_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* key_file);

/**
 * \brief Read a passphrase and use it to decrypt the certificate file.
 *
 * \param decrypted_cert     Pointer to buffer that will be initialized with the
 *                           decrypted certificate.
 * \param opts               The command-line options to use.
 * \param encrypted_cert     The encrypted certificate to decrypt.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_read_password_and_decrypt_certfile(
    vccrypt_buffer_t* decrypted_cert, commandline_opts* opts,
    const vccrypt_buffer_t* encrypted_cert);

/**
 * \brief Read the input certificate to be endorsed.
 *
 * \param cert               Pointer to buffer that will be initialized with the
 *                           certificate.
 * \param opts               The command-line options to use.
 * \param key_file           The certfile from which the key certificate is
 *                           read.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_read_input_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* input_file);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

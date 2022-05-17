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
#include <rcpr/rbtree.h>
#include <rcpr/uuid.h>
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

/** \brief An entry in the endorse uuid dictionary. */
typedef struct endorse_uuid_dictionary_entry endorse_uuid_dictionary_entry;

struct endorse_uuid_dictionary_entry
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    char* key;
    RCPR_SYM(rcpr_uuid) value;
};

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
 * \brief Get the endorse config file and output an error message if the endorse
 * config file option is not set on the command line.
 *
 * \param endorse_config_file   Pointer to receive the input file certfile
 *                              instance.
 * \param opts                  The command-line options to use.
 * \param alloc                 The allocator to use.
 * \param root                  The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_endorse_config_file(
    certfile** endorse_config_file, commandline_opts* opts,
    RCPR_SYM(allocator)* alloc, const root_command* root);

/**
 * \brief Get a pubkey certfile by name and output an error message if the file
 * could not be stat'ed.
 *
 * \param pubkey_file   Pointer to receive the pubkey file certfile instance.
 * \param opts          The command-line options to use.
 * \param alloc         The allocator to use.
 * \param filename      The name of the file to stat.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_pubkey_file(
    certfile** key_file, commandline_opts* opts, RCPR_SYM(allocator)* alloc,
    const char* filename);

/**
 * \brief Given a pubkey certfile, open it and parse it to obtain the entity
 * id.
 *
 * \param entity_id     \ref rcpr_uuid pointer to be populated with the entity
 *                      id.
 * \param pubkey_file   The pubkey certfile instance.
 * \param opts          The command-line options to use.
 * \param alloc         The allocator to use.
 * \param root          The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_pubkey_id(
    RCPR_SYM(rcpr_uuid)* entity_id, certfile* key_file, commandline_opts* opts,
    RCPR_SYM(allocator)* alloc, const root_command* root);

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

/**
 * \brief Read the endorse config file.
 *
 * \param cert                  Pointer to buffer that will be initialized with
 *                              the endorse config data.
 * \param opts                  The command-line options to use.
 * \param endorse_config_file   The certfile from which the endorse config is
 *                              read.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_read_endorse_config_file(
    vccrypt_buffer_t* cert, commandline_opts* opts,
    const certfile* endorse_config_file);

/**
 * \brief Build a map of key to UUID using the command-line options.
 *
 * \param dict              Receive a pointer to the dictionary on success.
 * \param alloc             The allocator to use for this operation.
 * \param opts              The command-line options to use.
 * \param root              The root command config.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_build_uuid_dictionary(
    RCPR_SYM(rbtree)** dict, RCPR_SYM(allocator)* alloc, commandline_opts* opts,
    const root_command* root);

/**
 * \brief Compare two opaque uuid values.
 *
 * \param context       Unused.
 * \param lhs           The left-hand side of the comparison.
 * \param rhs           The right-hand side of the comparison.
 *
 * \returns an integer value representing the comparison result.
 *      - RCPR_COMPARE_LT if \p lhs &lt; \p rhs.
 *      - RCPR_COMPARE_EQ if \p lhs == \p rhs.
 *      - RCPR_COMPARE_GT if \p lhs &gt; \p rhs.
 */
RCPR_SYM(rcpr_comparison_result) endorse_uuid_dictionary_compare(
    void* /*context*/, const void* lhs, const void* rhs);

/**
 * \brief Given an endorse_uuid_dictionary_entry, return the key.
 *
 * \param context       Unused.
 * \param r             The resource handle of the
 *                      endorse_uuid_dictionary_entry.
 *
 * \returns the key for the entry.
 */
const void* endorse_uuid_dictionary_key(
    void* /*context*/, const RCPR_SYM(resource)* r);

/**
 * \brief Add an entry to the uuid dictionary.
 *
 * \param dict              The dictionary to which this entry is added.
 * \param alloc             The allocator to use for this operation.
 * \param key               The key for this entry.
 * \param value             The UUID value for this entry.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_uuid_dictionary_add(
    RCPR_SYM(rbtree)* dict, RCPR_SYM(allocator)* alloc, const char* key,
    const RCPR_SYM(rcpr_uuid)* value);

/**
 * \brief Release an endorse uuid dictionary entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_uuid_dictionary_entry_resource_release(RCPR_SYM(resource)* r);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

/**
 * \file command/endorse/endorse_get_pubkey_id.c
 *
 * \brief Get the entity id from a public key file.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <vccert/fields.h>
#include <vccert/parser.h>
#include <vpr/parameters.h>

#include "endorse_internal.h"

RCPR_IMPORT_uuid;

/* forward decls */
static bool dummy_txn_resolver(
    void* options, void* parser, const uint8_t* artifact_id,
    const uint8_t* txn_id, vccrypt_buffer_t* output_buffer, bool* trusted);
static int32_t dummy_artifact_state_resolver(
    void* options, void* parser, const uint8_t* artifact_id,
    vccrypt_buffer_t* txn_id);
static int dummy_contract_resolver(
    void* options, void* parser, const uint8_t* type_id,
    const uint8_t* artifact_id,
    vccert_contract_closure_t* closure);
static bool dummy_key_resolver(
    void* options, void* parser, uint64_t height, const uint8_t* entity_id,
    vccrypt_buffer_t* pubenckey_buffer, vccrypt_buffer_t* pubsignkey_buffer);

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
    RCPR_SYM(allocator)*, const root_command*)
{
    status retval, release_retval;
    vccrypt_buffer_t file_buffer;
    vccert_parser_options_t parser_opts;
    vccert_parser_context_t parser;
    int fd;

    /* create the certificate buffer. */
    retval =
        vccrypt_buffer_init(
            &file_buffer, opts->suite->alloc_opts, key_file->size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Out of memory.\n");
        goto done;
    }

    /* open the file. */
    retval = file_open(opts->file, &fd, key_file->filename, O_RDONLY, 0);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Error opening file %s for read.\n", key_file->filename);
        goto cleanup_file_buffer;
    }

    /* read the contents into the certificate buffer. */
    size_t read_bytes;
    retval =
        file_read(
            opts->file, fd, file_buffer.data, file_buffer.size, &read_bytes);
    if (STATUS_SUCCESS != retval || read_bytes != file_buffer.size)
    {
        fprintf(stderr, "Error reading from %s.\n", key_file->filename);
        goto cleanup_fd;
    }

    /* create parser options. */
    retval =
        vccert_parser_options_init(
            &parser_opts, opts->suite->alloc_opts, opts->suite,
            &dummy_txn_resolver, &dummy_artifact_state_resolver,
            &dummy_contract_resolver, &dummy_key_resolver, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_fd;
    }

    /* create a parser instance backed by the file buffer. */
    retval =
        vccert_parser_init(
            &parser_opts, &parser, file_buffer.data, file_buffer.size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser_opts;
    }

    /* read the artifact uuid. */
    const uint8_t* artifact_id;
    size_t artifact_id_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_ARTIFACT_ID, &artifact_id,
            &artifact_id_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* verify that the artifact id size is correct. */
    if (artifact_id_size != sizeof(rcpr_uuid))
    {
        retval = VCCERT_ERROR_PARSER_FIND_NEXT_INVALID_FIELD_SIZE;
        goto cleanup_parser;
    }

    /* Success. Copy the artifact id. */
    retval = STATUS_SUCCESS;
    memcpy(entity_id, artifact_id, artifact_id_size);
    goto cleanup_parser;

cleanup_parser:
    dispose((disposable_t*)&parser);

cleanup_parser_opts:
    dispose((disposable_t*)&parser_opts);

cleanup_fd:
    release_retval = file_close(opts->file, fd);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_file_buffer:
    dispose(vccrypt_buffer_disposable_handle(&file_buffer));

done:
    return retval;
}

/**
 * \brief Dummy transaction resolver.
 */
static bool dummy_txn_resolver(
    void* UNUSED(options), void* UNUSED(parser),
    const uint8_t* UNUSED(artifact_id),
    const uint8_t* UNUSED(txn_id), vccrypt_buffer_t* UNUSED(output_buffer),
    bool* UNUSED(trusted))
{
    return false;
}

/**
 * \brief Dummy artifact state resolver.
 */
static int32_t dummy_artifact_state_resolver(
    void* UNUSED(options), void* UNUSED(parser),
    const uint8_t* UNUSED(artifact_id), vccrypt_buffer_t* UNUSED(txn_id))
{
    return 0;
}

/**
 * \brief Dummy contract resolver.
 */
static int dummy_contract_resolver(
    void* UNUSED(options), void* UNUSED(parser), const uint8_t* UNUSED(type_id),
    const uint8_t* UNUSED(artifact_id),
    vccert_contract_closure_t* UNUSED(closure))
{
    return VCCERT_ERROR_PARSER_ATTEST_MISSING_CONTRACT;
}

/**
 * \brief Dummy key resolver.
 */
static bool dummy_key_resolver(
    void* UNUSED(options), void* UNUSED(parser), uint64_t UNUSED(height),
    const uint8_t* UNUSED(entity_id),
    vccrypt_buffer_t* UNUSED(pubenckey_buffer),
    vccrypt_buffer_t* UNUSED(pubsignkey_buffer))
{
    return false;
}

/**
 * \file command/endorse/endorse_command_func.c
 *
 * \brief Entry point for the endorse command.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <vctool/endorse.h>

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

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
    status retval, release_retval;
    certfile* key_file;
    certfile* input_file;
    certfile* endorse_config_file;
    char* output_filename;
    vccrypt_buffer_t key_cert;
    vccrypt_buffer_t input_cert;
    vccrypt_buffer_t endorse_cfg;
    endorse_config_context* endorse_ctx;
    const endorse_config* ast;
    rbtree* dict;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* get endorse and root command. */
    endorse_command* endorse = (endorse_command*)opts->cmd;
    MODEL_ASSERT(NULL != endorse);
    root_command* root = (root_command*)endorse->hdr.next;
    MODEL_ASSERT(NULL != root);

    /* get the key filename. */
    TRY_OR_FAIL(endorse_get_key_file(&key_file, opts, root->alloc, root), done);

    /* get the input filename. */
    TRY_OR_FAIL(
        endorse_get_input_file(
            &input_file, opts, root->alloc, root),
        cleanup_key_file);

    /* get the endorse config filename. */
    TRY_OR_FAIL(
        endorse_get_endorse_config_file(
            &endorse_config_file, opts, root->alloc, root),
        cleanup_input_file);

    /* get the output filename. */
    TRY_OR_FAIL(
        endorse_get_output_filename(
            &output_filename, opts, input_file->filename, root),
        cleanup_endorse_config_file);

    /* Verify that the endorser private key is valid and read it. */
    TRY_OR_FAIL(
        endorse_read_key_certificate(&key_cert, opts, key_file),
        cleanup_output_filename);

    /* Verify that the input public key file is valid and read it. */
    TRY_OR_FAIL(
        endorse_read_input_certificate(&input_cert, opts, input_file),
        cleanup_key_cert);

    /* create the endorse config context. */
    TRY_OR_FAIL(
        endorse_config_create_default(&endorse_ctx, root->alloc),
        cleanup_input_cert);

    /* Read the endorse config file. */
    TRY_OR_FAIL(
        endorse_read_endorse_config_file(
            &endorse_cfg, opts, endorse_config_file),
        cleanup_endorse_ctx);

    /* parse the endorse config file. */
    TRY_OR_FAIL(
        endorse_parse(endorse_ctx, &endorse_cfg),
        cleanup_endorse_cfg);

    /* get the root config. */
    ast = endorse_config_default_context_get_endorse_config_root(endorse_ctx);

    /* perform semantic analysis on the endorse config. */
    TRY_OR_FAIL(
        endorse_analyze(endorse_ctx, (endorse_config*)ast),
        cleanup_endorse_cfg);

    /* build a dictionary of dictionary key to entity UUID data. */
    TRY_OR_FAIL(
        endorse_build_uuid_dictionary(&dict, root->alloc, opts, root),
        cleanup_endorse_cfg);

    /* For each dictionary definition: */
        /* Verify that the definition is an entity in the config. */
        /* Verify that the file is a valid public key file and read it. */
        /* Save the UUID for the entity to the AST. */
    /* For each permission: */
        /* Verify that the entity exists in the AST and has a UUID. */
        /* Expand the moiety into a set of verbs. */
        /* For each verb, create a pair of entity id / verb id. */
        /* See if this pair exists in the working set. */
        /* If not, add it. */
    /* Create the output file. */
    /* Append all public key data from the input file. */
    /* For each pair in the working set: */
        /* Add an endorsement permission triplet to the output file. */
    /* Sign the output file using the endorser signing key and id. */

    fprintf(stderr, "endorse not yet implemented.\n");

    goto cleanup_dict;

cleanup_dict:
    CLEANUP_OR_CASCADE(rbtree_resource_handle(dict));

cleanup_endorse_cfg:
    dispose(vccrypt_buffer_disposable_handle(&endorse_cfg));

cleanup_endorse_ctx:
    CLEANUP_OR_CASCADE(&endorse_ctx->hdr);

cleanup_input_cert:
    dispose(vccrypt_buffer_disposable_handle(&input_cert));

cleanup_key_cert:
    dispose(vccrypt_buffer_disposable_handle(&key_cert));

cleanup_output_filename:
    free(output_filename);

cleanup_endorse_config_file:
    CLEANUP_OR_CASCADE(&endorse_config_file->hdr);

cleanup_input_file:
    CLEANUP_OR_CASCADE(&input_file->hdr);

cleanup_key_file:
    CLEANUP_OR_CASCADE(&key_file->hdr);

done:
    return retval;
}

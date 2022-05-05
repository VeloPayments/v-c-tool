/**
 * \file command/endorse/endorse_command_func.c
 *
 * \brief Entry point for the endorse command.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/* forward decls. */
static status read_input_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* input_file);

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
    char* output_filename;
    vccrypt_buffer_t key_cert;
    vccrypt_buffer_t input_cert;

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

    /* get the output filename. */
    TRY_OR_FAIL(
        endorse_get_output_filename(
            &output_filename, opts, input_file->filename, root),
        cleanup_input_file);

    /* Verify that the endorser private key is valid and read it. */
    TRY_OR_FAIL(
        endorse_read_key_certificate(&key_cert, opts, key_file),
        cleanup_output_filename);

    /* Verify that the input public key file is valid and read it. */
    TRY_OR_FAIL(
        read_input_certificate(&input_cert, opts, input_file),
        cleanup_key_cert);

    /* Read / parse the endorse config file. */
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

    (void)opts;

    fprintf(stderr, "endorse not yet implemented.\n");

    goto cleanup_input_cert;

cleanup_input_cert:
    dispose(vccrypt_buffer_disposable_handle(&input_cert));

cleanup_key_cert:
    dispose(vccrypt_buffer_disposable_handle(&key_cert));

cleanup_output_filename:
    free(output_filename);

cleanup_input_file:
    CLEANUP_OR_CASCADE(&input_file->hdr);

cleanup_key_file:
    CLEANUP_OR_CASCADE(&key_file->hdr);

done:
    return retval;
}

/**
 * \brief Read the input file.
 */
static status read_input_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* input_file)
{
    status retval, release_retval;
    int fd;
    bool save_cert = false;

    /* create the certificate buffer. */
    retval =
        vccrypt_buffer_init(cert, opts->suite->alloc_opts, input_file->size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Out of memory.\n");
        goto done;
    }

    /* open the file. */
    retval =
        file_open(
            opts->file, &fd, input_file->filename, O_RDONLY, 0);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Error opening file %s for read.\n", input_file->filename);
        goto cleanup_cert;
    }

    /* read the contents into the certificate buffer. */
    size_t read_bytes;
    retval = file_read(opts->file, fd, cert->data, cert->size, &read_bytes);
    if (STATUS_SUCCESS != retval || read_bytes != cert->size)
    {
        fprintf(stderr, "Error reading from %s.\n", input_file->filename);
        goto cleanup_file;
    }

    /* success. Instruct our cleanup logic to save the certificate. */
    save_cert = true;
    retval = STATUS_SUCCESS;
    goto cleanup_file;

cleanup_file:
    release_retval = file_close(opts->file, fd);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_cert:
    /* dispose of the certificate unless we are saving it for the caller. */
    if (retval != STATUS_SUCCESS || !save_cert)
    {
        dispose(vccrypt_buffer_disposable_handle(cert));
    }

done:
    return retval;
}

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
static status get_output_filename(
    char** output_filename, commandline_opts* opts, const char* input_filename,
    const root_command* root);
static status read_key_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* key_file);
static status read_password_and_decrypt_certfile(
    vccrypt_buffer_t* decrypted_cert, commandline_opts* opts,
    const vccrypt_buffer_t* encrypted_cert);
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
        get_output_filename(&output_filename, opts, input_file->filename, root),
        cleanup_input_file);

    /* Verify that the endorser private key is valid and read it. */
    TRY_OR_FAIL(
        read_key_certificate(&key_cert, opts, key_file),
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
 * \brief Compute the output filename and output an error message if unset.
 */
static status get_output_filename(
    char** output_filename, commandline_opts* opts, const char* input_filename,
    const root_command* root)
{
    status retval;

    /* if the output file is set, use it. */
    if (NULL != root->output_filename)
    {
        *output_filename = strdup(root->output_filename);
        retval = STATUS_SUCCESS;
        goto done;
    }

    /* compute the filename length. */
    size_t output_filename_length =
        strlen(input_filename)
      + 9 /* .endorsed */
      + 1;/* asciiz */

    /* allocate memory for the filename. */
    *output_filename = (char*)malloc(output_filename_length);
    if (NULL == *output_filename)
    {
        fprintf(stderr, "Out of memory.\n");
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* create the output filename. */
    memset(*output_filename, 0, output_filename_length);
    snprintf(
        *output_filename, output_filename_length, "%s.endorsed",
        input_filename);

    /* stat the output file to ensure it does not exist. */
    file_stat_st fst;
    retval = file_stat(opts->file, *output_filename, &fst);
    if (VCTOOL_ERROR_FILE_NO_ENTRY != retval)
    {
        fprintf(
            stderr, "Won't clobber existing file %s.  Stopping.\n",
            *output_filename);
        retval = VCTOOL_ERROR_ENDORSE_WOULD_CLOBBER_FILE;
        goto cleanup_output_filename;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_output_filename:
    free(*output_filename);

done:
    return retval;
}

/**
 * \brief Read the key certificate file.
 */
static status read_key_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* key_file)
{
    status retval, release_retval;
    int fd;
    vccrypt_buffer_t tmp;
    bool save_cert = false;

    /* create the certificate buffer. */
    retval = vccrypt_buffer_init(cert, opts->suite->alloc_opts, key_file->size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Out of memory.\n");
        goto done;
    }

    /* open the file. */
    retval =
        file_open(
            opts->file, &fd, key_file->filename, O_RDONLY, 0);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Error opening file %s for read.\n", key_file->filename);
        goto cleanup_cert;
    }

    /* read the contents into the certificate buffer. */
    size_t read_bytes;
    retval = file_read(opts->file, fd, cert->data, cert->size, &read_bytes);
    if (STATUS_SUCCESS != retval || read_bytes != cert->size)
    {
        fprintf(stderr, "Error reading from %s.\n", key_file->filename);
        goto cleanup_file;
    }

    /* Does it have encryption magic? */
    if (cert->size > ENCRYPTED_CERT_MAGIC_SIZE
     && !crypto_memcmp(
            cert->data, ENCRYPTED_CERT_MAGIC_STRING, ENCRYPTED_CERT_MAGIC_SIZE))
    {
        /* Yes: read password and decrypt file. */
        retval = read_password_and_decrypt_certfile(&tmp, opts, cert);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_file;
        }

        /* dispose of the encrypted certificate. */
        dispose(vccrypt_buffer_disposable_handle(cert));

        /* move the decrypted certificate to the certificate buffer. */
        vccrypt_buffer_move(cert, &tmp);
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

/**
 * \brief Read a password from the user and use it to decrypt the certificate
 * file.
 */
static status read_password_and_decrypt_certfile(
    vccrypt_buffer_t* decrypted_cert, commandline_opts* opts,
    const vccrypt_buffer_t* encrypted_cert)
{
    status retval;
    vccrypt_buffer_t password_buffer;
    vccrypt_buffer_t* tmp;

    /* Read the passphrase. */
    printf("Enter passphrase: ");
    fflush(stdout);
    retval = readpassword(opts->suite, &password_buffer);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Failure.\n");
        goto done;
    }
    printf("\n");

    /* decrypt the certificate. */
    retval =
        certificate_decrypt(
            opts->suite, &tmp, encrypted_cert, &password_buffer);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error decrypting key file.\n");
        goto cleanup_passphrase;
    }

    /* success. */
    vccrypt_buffer_move(decrypted_cert, tmp);
    free(tmp);
    retval = STATUS_SUCCESS;
    goto cleanup_passphrase;

cleanup_passphrase:
    dispose(vccrypt_buffer_disposable_handle(&password_buffer));

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

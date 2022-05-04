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
#include <string.h>
#include <vccrypt/compare.h>
#include <vctool/certificate.h>
#include <vctool/command/endorse.h>
#include <vctool/command/root.h>
#include <vctool/control.h>
#include <vctool/readpassword.h>

#include "certfile.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/* forward decls. */
static status get_key_file(
    certfile** key_file, commandline_opts* opts, rcpr_allocator* alloc,
    const root_command* root);
static status get_input_file(
    certfile** input_file, commandline_opts* opts, rcpr_allocator* alloc,
    const root_command* root);
static status get_output_filename(
    char** output_filename, const char* input_filename,
    const root_command* root);
static status read_key_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* key_file);
static status read_password_and_decrypt_certfile(
    vccrypt_buffer_t* decrypted_cert, commandline_opts* opts,
    const vccrypt_buffer_t* encrypted_cert);

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

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* get endorse and root command. */
    endorse_command* endorse = (endorse_command*)opts->cmd;
    MODEL_ASSERT(NULL != endorse);
    root_command* root = (root_command*)endorse->hdr.next;
    MODEL_ASSERT(NULL != root);

    /* get the key filename. */
    TRY_OR_FAIL(get_key_file(&key_file, opts, root->alloc, root), done);

    /* get the input filename. */
    TRY_OR_FAIL(
        get_input_file(&input_file, opts, root->alloc, root), cleanup_key_file);

    /* get the output filename. */
    TRY_OR_FAIL(
        get_output_filename(&output_filename, input_file->filename, root),
        cleanup_input_file);

    /* Verify that the endorser private key is valid and read it. */
    TRY_OR_FAIL(
        read_key_certificate(&key_cert, opts, key_file),
        cleanup_output_filename);

    /* Verify that the input public key file is valid and read it. */
    /* Verify that the output file won't clobber an existing file. */
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

    goto cleanup_key_cert;

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
 * \brief Key the key filename and output an error message if unset.
 */
static status get_key_file(
    certfile** key_file, commandline_opts* opts, rcpr_allocator* alloc,
    const root_command* root)
{
    status retval;
    file_stat_st fst;

    /* check the key filename. */
    if (NULL == root->key_filename)
    {
        fprintf(stderr, "Expecting a key filename (-k endorser.cert).\n");
        retval = VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT;
        goto done;
    }

    /* make sure the key file exists. */
    retval = file_stat(opts->file, root->key_filename, &fst);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Missing key file %s.\n", root->key_filename);
        goto done;
    }

    /* make sure the permission bits are set appropriately. */
    mode_t bad_bits = S_ISUID | S_ISGID | S_ISVTX | S_IRWXG | S_IRWXO;
    if (fst.fst_mode & bad_bits)
    {
        fprintf(
            stderr, "Only user permissions allowed for %s.\n",
            root->key_filename);
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_FILE_PERMISSIONS;
        goto done;
    }
    else if (! (fst.fst_mode & S_IRUSR))
    {
        fprintf(stderr, "Can't read %s.\n", root->key_filename);
        goto done;
    }

    /* create the key certfile. */
    retval = certfile_create(key_file, alloc, root->key_filename, fst.fst_size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Can't create certfile for %s.\n", root->key_filename);
        goto done;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

/**
 * \brief Key the input filename and output an error message if unset.
 */
static status get_input_file(
    certfile** input_file, commandline_opts* opts, rcpr_allocator* alloc,
    const root_command* root)
{
    status retval;
    file_stat_st fst;

    /* check the input filename. */
    if (NULL == root->input_filename)
    {
        fprintf(stderr, "Expecting an input filename (-i user.pub).\n");
        retval = VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT;
        goto done;
    }

    /* make sure the input file exists. */
    retval = file_stat(opts->file, root->input_filename, &fst);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Missing input file %s.\n", root->input_filename);
        goto done;
    }

    /* create the input certfile. */
    retval =
        certfile_create(input_file, alloc, root->input_filename, fst.fst_size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Can't create certfile for %s.\n", root->input_filename);
        goto done;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

/**
 * \brief Key the output filename and output an error message if unset.
 */
static status get_output_filename(
    char** output_filename, const char* input_filename,
    const root_command* root)
{
    if (NULL != root->output_filename)
    {
        *output_filename = strdup(root->output_filename);
        return STATUS_SUCCESS;
    }
    else
    {
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
            return VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        }

        /* create the output filename. */
        memset(*output_filename, 0, output_filename_length);
        snprintf(
            *output_filename, output_filename_length, "%s.endorsed",
            input_filename);

        return STATUS_SUCCESS;
    }
}

/**
 * \brief Read the key certificate file.
 */
static status read_key_certificate(
    vccrypt_buffer_t* cert, commandline_opts* opts, const certfile* key_file)
{
    status retval;
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
    file_close(opts->file, fd);

cleanup_cert:
    /* dispose of the certificate unless we are saving it for the caller. */
    if (!save_cert)
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

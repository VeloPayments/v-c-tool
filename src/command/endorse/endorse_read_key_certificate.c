/**
 * \file command/endorse/endorse_read_key_certificate.c
 *
 * \brief Read the key certificate file and optionally prompt for a passphrase
 * and decrypt it.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

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
        retval = endorse_read_password_and_decrypt_certfile(&tmp, opts, cert);
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

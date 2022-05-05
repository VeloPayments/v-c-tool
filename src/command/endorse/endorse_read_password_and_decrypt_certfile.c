/**
 * \file command/endorse/endorse_read_password_and_decrypt_certfile.c
 *
 * \brief Read a passphrase and use it to decrypt the certificate.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

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

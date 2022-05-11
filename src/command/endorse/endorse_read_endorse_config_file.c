/**
 * \file command/endorse/endorse_read_endorse_config_file.c
 *
 * \brief Read the endorse config file.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

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
    const certfile* endorse_config_file)
{
    status retval, release_retval;
    int fd;
    bool save_cert = false;

    /* create the certificate buffer. */
    retval =
        vccrypt_buffer_init(
            cert, opts->suite->alloc_opts, endorse_config_file->size + 1);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Out of memory.\n");
        goto done;
    }

    /* clear the buffer. */
    memset(cert->data, 0, cert->size);

    /* open the file. */
    retval =
        file_open(
            opts->file, &fd, endorse_config_file->filename, O_RDONLY, 0);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Error opening file %s for read.\n",
            endorse_config_file->filename);
        goto cleanup_cert;
    }

    /* read the contents into the buffer. */
    size_t read_bytes;
    retval =
        file_read(
            opts->file, fd, cert->data, endorse_config_file->size, &read_bytes);
    if (STATUS_SUCCESS != retval || read_bytes != endorse_config_file->size)
    {
        fprintf(
            stderr, "Error reading from %s.\n", endorse_config_file->filename);
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

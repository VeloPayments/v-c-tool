/**
 * \file command/endorse/endorse_get_pubkey_file.c
 *
 * \brief Get a pubkey certfile by name.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

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
    const char* filename)
{
    status retval;
    file_stat_st fst;

    /* make sure the pubkey file exists. */
    retval = file_stat(opts->file, filename, &fst);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Missing pubkey file %s.\n", filename);
        goto done;
    }

    /* create the input certfile. */
    retval =
        certfile_create(key_file, alloc, filename, fst.fst_size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Can't create certfile for %s.\n", filename);
        goto done;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

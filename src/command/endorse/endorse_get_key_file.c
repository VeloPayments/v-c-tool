/**
 * \file command/endorse/endorse_get_key_file.c
 *
 * \brief Get the key certfile from the command-line.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

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

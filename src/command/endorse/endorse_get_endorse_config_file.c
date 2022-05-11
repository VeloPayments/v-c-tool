/**
 * \file command/endorse/endorse_get_endorse_config_file.c
 *
 * \brief Get the endorse config file from the command-line.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Get the endorse config file and output an error message if the endorse
 * config file option is not set on the command line.
 *
 * \param endorse_config_file   Pointer to receive the input file certfile
 *                              instance.
 * \param opts                  The command-line options to use.
 * \param alloc                 The allocator to use.
 * \param root                  The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_endorse_config_file(
    certfile** endorse_config_file, commandline_opts* opts,
    RCPR_SYM(allocator)* alloc, const root_command* root)
{
    status retval;
    file_stat_st fst;

    /* check the endorse config filename. */
    if (NULL == root->endorse_config_filename)
    {
        fprintf(
            stderr, "Expecting an endorse config filename (-E endorse.cfg).\n");
        retval = VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT;
        goto done;
    }

    /* make sure the file exists. */
    retval = file_stat(opts->file, root->endorse_config_filename, &fst);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Missing config file %s.\n", root->endorse_config_filename);
        goto done;
    }

    /* create the endorse config certfile. */
    retval =
        certfile_create(
            endorse_config_file, alloc, root->endorse_config_filename,
            fst.fst_size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Can't create certfile for %s.\n",
            root->endorse_config_filename);
        goto done;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

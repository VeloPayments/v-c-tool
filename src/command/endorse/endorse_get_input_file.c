/**
 * \file command/endorse/endorse_get_input_file.c
 *
 * \brief Get the input certfile from the command-line.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Get the input file and output an error message if the input file
 * option is not set on the command line.
 *
 * \param input_file    Pointer to receive the input file certfile instance.
 * \param opts          The command-line options to use.
 * \param alloc         The allocator to use.
 * \param root          The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_input_file(
    certfile** input_file, commandline_opts* opts, RCPR_SYM(allocator)* alloc,
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

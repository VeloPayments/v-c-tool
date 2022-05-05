/**
 * \file command/endorse/endorse_get_output_filename.c
 *
 * \brief Get the output filename either from the command-line, or by using the
 * input filename as a template.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

/**
 * \brief Get the output filename and output an error message if the output file
 * would clobber an existing file.
 *
 * \param output_filename    Pointer to receive the output filename.
 * \param opts               The command-line options to use.
 * \param input_filename     The input filename to use as a template if an
 *                           output filename is not specified on the
 *                           command-line.
 * \param root               The root command instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_output_filename(
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

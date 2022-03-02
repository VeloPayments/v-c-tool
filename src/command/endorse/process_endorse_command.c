/**
 * \file command/endorse/process_endorse_command.c
 *
 * \brief Process command-line options to build an endorse command.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/command/endorse.h>
#include <vctool/command/root.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Process the endorse command.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int process_endorse_command(
    commandline_opts* opts, int UNUSED(argc), char* UNUSED(argv[]))
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* allocate memory for a endorse_command structure. */
    endorse_command* endorse =
        (endorse_command*)malloc(sizeof(endorse_command));
    if (NULL == endorse)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* initialize the structure. */
    retval = endorse_command_init(endorse);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto free_endorse;
    }

    /* set keygen command as the head of opts command. */
    endorse->hdr.next = opts->cmd;
    opts->cmd = &endorse->hdr;

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;
    goto done;

free_endorse:
    free(endorse);

done:
    return retval;
}

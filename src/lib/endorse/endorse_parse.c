/**
 * \file endorse/endorse_parse.c
 *
 * \brief Parse endorse config data from the given input buffer.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <stdio.h>
#include <vctool/endorse.h>

#include "endorse.tab.h"
#include "endorse.yy.h"

/**
 * \brief Parse a config file read into memory as a buffer.
 *
 * \param context       The endorse config context for this parse.
 * \param input         The input buffer to parse. Must be ASCIIZ.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_parse(
    endorse_config_context* context, const vccrypt_buffer_t* input)
{
    status retval;
    YY_BUFFER_STATE state;
    yyscan_t scanner;

    /* initialize the scanner. */
    retval = yylex_init(&scanner);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* initialize the state. */
    state = yy_scan_string(input->data, scanner);
    if (NULL == state)
    {
        goto cleanup_scanner;
    }

    /* parse the buffer. */
    retval = yyparse(scanner, context);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_buffer;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_buffer;

cleanup_buffer:
    yy_delete_buffer(state, scanner);

cleanup_scanner:
    yylex_destroy(scanner);

done:
    return retval;
}

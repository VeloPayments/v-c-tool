/**
 * \file readpassword/blankpassword.c
 *
 * \brief Function to create a blank password.
 *
 * \copyright 2021 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <vctool/commandline.h>
#include <vctool/readpassword.h>
#include <vpr/parameters.h>

/**
 * \brief In non-interactive mode, create a blank password.
 *
 * \param suite             The crypto suite to use to create the blank pwd.
 * \param passbuffer        Pointer to a vccrypt_buffer_t to be initialized with
 *                          the blank password.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int blankpassword(vccrypt_suite_options_t* suite, vccrypt_buffer_t* passbuffer)
{
    int retval;

    /* allocate the password buffer. */
    retval =
        vccrypt_buffer_init(
            passbuffer, suite->alloc_opts, 0);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* success. */
    retval = 0;
    goto done;

done:
    return retval;
}

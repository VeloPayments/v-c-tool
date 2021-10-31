/**
 * \file include/vctool/readpassword.h
 *
 * \brief Function to read a password without echoing.
 *
 * \copyright 2020-2021 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_READ_PASSWORD_HEADER_GUARD
# define VCTOOL_READ_PASSWORD_HEADER_GUARD

#include <vccrypt/buffer.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read a password from standard input.
 *
 * \param suite             The crypto suite to use to read the password.
 * \param passbuffer        Pointer to a vccrypt_buffer_t to be initialized with
 *                          the password / passphrase that has been read.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int readpassword(vccrypt_suite_options_t* suite, vccrypt_buffer_t* passbuffer);

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
int blankpassword(vccrypt_suite_options_t* suite, vccrypt_buffer_t* passbuffer);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_READ_PASSWORD_HEADER_GUARD*/

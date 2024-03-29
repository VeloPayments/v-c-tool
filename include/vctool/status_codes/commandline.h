/**
 * \file include/vctool/status_codes/commandline.h
 *
 * \brief commandline component status codes.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_COMMANDLINE_HEADER_GUARD
#define VCTOOL_STATUS_CODES_COMMANDLINE_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Missing argument.
 */
#define VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0001U)

/**
 * \brief Unknown command.
 */
#define VCTOOL_ERROR_COMMANDLINE_UNKNOWN_COMMAND \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0002U)

/**
 * \brief Missing command.
 */
#define VCTOOL_ERROR_COMMANDLINE_MISSING_COMMAND \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0003U)

/**
 * \brief Duplicate option.
 */
#define VCTOOL_ERROR_COMMANDLINE_DUPLICATE_OPTION \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0004U)

/**
 * \brief Invalid number of key rounds.
 */
#define VCTOOL_ERROR_COMMANDLINE_BAD_KEY_ROUNDS \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0005U)

/**
 * \brief Bad key-value pair.
 */
#define VCTOOL_ERROR_COMMANDLINE_BAD_KVP \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0006U)

/**
 * \brief Bad function parameter.
 */
#define VCTOOL_ERROR_COMMANDLINE_BAD_PARAMETER \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0007U)

/**
 * \brief Bad permission.
 */
#define VCTOOL_ERROR_COMMANDLINE_BAD_PERMISSION \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0008U)

/**
 * \brief Bad file permissions.
 */
#define VCTOOL_ERROR_COMMANDLINE_BAD_FILE_PERMISSIONS \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_COMMANDLINE, 0x0009U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_COMMANDLINE_HEADER_GUARD*/

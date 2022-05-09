/**
 * \file include/vctool/status_codes/endorse.h
 *
 * \brief Status codes for the endorse component.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_ENDORSE_HEADER_GUARD
#define VCTOOL_STATUS_CODES_ENDORSE_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The endorse command would clobber a file.
 */
#define VCTOOL_ERROR_ENDORSE_WOULD_CLOBBER_FILE \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_ENDORSE, 0x0001U)

/**
 * \brief The error message index is out of bounds.
 */
#define VCTOOL_ERROR_ENDORSE_ERROR_MESSAGE_INDEX_OUT_OF_BOUNDS \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_ENDORSE, 0x0002U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_ENDORSE_HEADER_GUARD*/

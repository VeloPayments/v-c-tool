/**
 * \file include/vctool/status_codes/pubkey.h
 *
 * \brief Status codes for the pubkey component.
 *
 * \copyright 2023 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_PUBKEY_HEADER_GUARD
#define VCTOOL_STATUS_CODES_PUBKEY_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The pubkey command would clobber a file.
 */
#define VCTOOL_ERROR_PUBKEY_WOULD_CLOBBER_FILE \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_PUBKEY, 0x0001U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_PUBKEY_HEADER_GUARD*/

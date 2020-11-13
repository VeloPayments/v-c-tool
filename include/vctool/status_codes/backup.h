/**
 * \file include/vctool/status_codes/backup.h
 *
 * \brief Status codes for the backup component.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_BACKUP_HEADER_GUARD
#define VCTOOL_STATUS_CODES_BACKUP_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief An invalid parameter was passed to a backup function.
 */
#define VCTOOL_ERROR_BACKUP_BAD_PARAMETER \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_BACKUP, 0x0001U)

/**
 * \brief A record was truncated when written.
 */
#define VCTOOL_ERROR_BACKUP_TRUNCATED_RECORD \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_BACKUP, 0x0002U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_BACKUP_HEADER_GUARD*/

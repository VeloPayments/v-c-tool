/**
 * \file file/file_fsync.c
 *
 * \brief Implementation of file_fsync.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>

/**
 * \brief Synchronize the file and data, blocking until the sync is complete.
 *
 * \param f         The file interface.
 * \param d         The descriptor to be synchronized with the filesystem.
 *
 * \note that if \ref VCTOOL_ERROR_FILE_IO, VCTOOL_ERROR_FILE_NO_SPACE, or
 * VCTOOL_ERROR_FILE_QUOTA or \ref VCTOOL_ERROR_FILE_UNKNOWN is returned, then
 * the process should probably panic and exit. On Linux and other flavors of
 * Unix, a synchronization error is likely fatal beyond the life of the process,
 * and possibly indicates bigger problems that require a human to solve. When
 * using \ref file_fsync, a strategy should be employed to make writes and
 * synchronization durable.
 * 
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is invalid.
 *      - VCTOOL_ERROR_FILE_IO if an error occurred during synchronization.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if disk space was exhausted while
 *        synchronizing.
 *      - VCTOOL_ERROR_FILE_INVALID if the descriptor is bound to a file or
 *        device that does not support synchronization.
 *      - VCTOOL_ERROR_FILE_QUOTA if a quota issue occurred.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error occurred.
 */
int file_fsync(file* f, int d)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);

    return f->file_fsync_method(f, d);
}

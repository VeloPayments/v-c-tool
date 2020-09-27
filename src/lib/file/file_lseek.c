/**
 * \file file/file_lseek.c
 *
 * \brief Implementation of file_lseek.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>

/**
 * \brief Reposition the read/write offset for a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor to be adjusted.
 * \param offset    The new offset.
 * \param whence    Indicates how the offset is interpreted.
 * \param newoffset Pointer to the updated offset.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is invalid.
 *      - VCTOOL_ERROR_FILE_INVALID if whence is invalid or if the resulting
 *        file offset would be negative or beyond the end of a seekable device.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if the resulting file offset cannot be
 *        represented as an off_t value.
 *      - VCTOOL_ERROR_FILE_BAD_ADDRESS if the offset for
 *        \ref FILE_LSEEK_WHENCE_DATA or \ref FILE_LSEEK_WHENCE_HOLE is beyond
 *        the end of the file.
 *      - VCTOOL_ERROR_FILE_IS_PIPE if the file is a pipe, socket, or FIFO,
 *        which is not seekable.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error has occurred.
 */
int file_lseek(
    file* f, int d, off_t offset, file_lseek_whence whence, off_t* newoffset)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);
    MODEL_ASSERT(
           FILE_LSEEK_WHENCE_ABSOLUTE == whence
        || FILE_LSEEK_WHENCE_CUR == whence
        || FILE_LSEEK_WHENCE_END == whence
        || FILE_LSEEK_WHENCE_DATA == whence
        || FILE_LSEEK_WHENCE_HOLE == whence);
    MODEL_ASSERT(NULL != newoffset);

    return f->file_lseek_method(f, d, offset, whence, newoffset);
}

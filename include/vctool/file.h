/**
 * \file include/vctool/file.h
 *
 * \brief File interrogation and I/O wrappers.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_FILE_HEADER_GUARD
# define VCTOOL_FILE_HEADER_GUARD

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vctool/status_codes.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/* forward decls */
typedef struct file_stat_st file_stat_st;
typedef struct file file;

/**
 * \brief This enumeration explains how a file offset should be interpreted in
 * \ref file_lseek.
 */
typedef enum file_lseek_whence
{
    /** \brief The file offset is set to the given absolute offset. */
    FILE_LSEEK_WHENCE_ABSOLUTE,

    /** \brief The file offset is set to the current location plus offset. */
    FILE_LSEEK_WHENCE_CUR,

    /** \brief The file offset is set to the size of the file plus offset. */
    FILE_LSEEK_WHENCE_END,

    /**
     * \brief Adjust the file offset to the next location in the file containing
     * data; this location is greater than or equal to the offset.
     */
    FILE_LSEEK_WHENCE_DATA,

    /**
     * \brief Adjust the file offset to the next hole in the file greater than
     * or equal to offset. If the offset points into the middle of a hole, then
     * the file offset is set to that offset. If there is no hole past this
     * offset, then the file offset is adjusted to the end of the file.
     */
    FILE_LSEEK_WHENCE_HOLE,

} file_lseek_whence;

/**
 * \brief File stats.
 */
struct file_stat_st
{
    mode_t fst_mode;
    uid_t fst_uid;
    gid_t fst_gid;
    off_t fst_size;
};

/**
 * \brief File operations.
 */
struct file
{
    /** \brief file is disposable. */
    disposable_t hdr;

    /** \brief stat method. */
    int (*file_stat_method)(file*, const char*, file_stat_st*);

    /** \brief open method. */
    int (*file_open_method)(file*, int*, const char*, int, mode_t);

    /** \brief close method. */
    int (*file_close_method)(file*, int);

    /** \brief read method. */
    int (*file_read_method)(file*, int, void*, size_t, size_t*);

    /** \brief write method. */
    int (*file_write_method)(file*, int, const void*, size_t, size_t*);

    /** \brief lseek method. */
    int (*file_lseek_method)(file*, int, off_t, file_lseek_whence, off_t*);

    /** \brief fsync method. */
    int (*file_fsync_method)(file*, int);

    /** \brief context structure. */
    void* context;
};

/**
 * \brief Initialize a file interface backed by the operating system.
 *
 * \param f         The file interface to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int file_init(file* f);

/**
 * \brief Get file stats.
 *
 * \param f         The file interface to use to gather stats.
 * \param path      Path to the file to stat.
 * \param filestat  Structure to populate with stat information.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_ACCESS if this failed due to permissions.
 *      - VCTOOL_ERROR_FILE_LOOP if too many symlinks were encountered.
 *      - VCTOOL_ERROR_FILE_NAME_TOO_LONG if the pathname is too long.
 *      - VCTOOL_ERROR_FILE_NO_ENTRY if a component of the pathname does not
 *        exist.
 *      - VCTOOL_ERROR_FILE_KERNEL_MEMORY if the kernel ran out of memory.
 *      - VCTOOL_ERROR_FILE_NOT_DIRECTORY if a component of the file path is not
 *        a directory.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if some value caused overflow.
 *      - VCTOOL_ERROR_FILE_UNKNOWN some unknown error occurred.
 */
int file_stat(file* f, const char* path, file_stat_st* filestat);
 
/**
 * \brief Open a file for I/O.
 *
 * \param f         The file interface.
 * \param d         Pointer to the descriptor to hold the open file.
 * \param path      Path to the file to open.
 * \param flags     Flags to use when opening the file.
 * \param mode      Mode to use when creating a file.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_ACCESS if permission or options make this file
 *        inacessible for the desired operation or mode.
 *      - VCTOOL_ERROR_FILE_QUOTA if this operation exceeds the quota for this
 *        user.
 *      - VCTOOL_ERROR_FILE_EXISTS if the file already exists and the options
 *        were to create a new file.
 *      - VCTOOL_ERROR_FILE_FAULT if a parameter passed to this method caused a
 *        read / write fault.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation was interrupted by a
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_INVALID_FLAGS if the flags provided were invalid for
 *        this file.
 *      - VCTOOL_ERROR_FILE_IS_DIRECTORY if an attempt was made to treat a
 *        directory as a file.
 *      - VCTOOL_ERROR_FILE_LOOP if too many symlinks were encountered during
 *        this operation.
 *      - VCTOOL_ERROR_FILE_TOO_MANY_FILES if too many files are open for this
 *        process, user, or the whole system.
 *      - VCTOOL_ERROR_FILE_NAME_TOO_LONG if the path name is too long.
 *      - VCTOOL_ERROR_FILE_NO_ENTRY if an attempt was made to access a file
 *        that does not exist.
 *      - VCTOOL_ERROR_FILE_KERNEL_MEMORY if the kernel ran out of memory.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if there is no space left on this device.
 *      - VCTOOL_ERROR_FILE_NOT_DIRECTORY if a component of the path is not a
 *        directory.
 *      - VCTOOL_ERROR_FILE_NOT_SUPPORTED if the options or name are not
 *        supported on this device.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if an overflow would occur in userspace due
 *        to this operation.
 *      - VCTOOL_ERROR_FILE_WOULD_BLOCK if this operation would block the
 *        process and no blocking is enabled.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error occurred.
 */
int file_open(file* f, int* d, const char* path, int flags, mode_t mode);

/**
 * \brief Close a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor to close.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is bad.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation was interrupted by the
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_IO if an I/O error occurred during this operation.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if there is no space left on the device.
 *      - VCTOOL_ERROR_FILE_QUOTA if user quota has been exceeded.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error has occurred.
 */
int file_close(file* f, int d);

/**
 * \brief Read from a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor from which to read.
 * \param buf       The buffer to read into.
 * \param max       The maximum number of bytes to read.
 * \param rbytes    Pointer to the size_t variable to hold the number of bytes
 *                  read.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_WOULD_BLOCK if this read would cause the process to
 *        block and no blocking is enabled.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is invalid.
 *      - VCTOOL_ERROR_FILE_FAULT if this operation causes a memory fault.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation is interrupted by a
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_INVALID_FLAGS if this read operation is invalid due
 *        to the way that this file descriptor was opened or due to the nature
 *        of the file or device.
 *      - VCTOOL_ERROR_FILE_IO if an I/O error has occurred.
 *      - VCTOOL_ERROR_FILE_IS_DIRECTORY if an attempt to perform a normal read
 *        on a directory file descriptor has been made.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error has occurred.
 */
int file_read(file* f, int d, void* buf, size_t max, size_t* rbytes);

/**
 * \brief Write to a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor to which data is written.
 * \param buf       The buffer to write from.
 * \param max       The maximum number of bytes to write.
 * \param wbytes    Pointer to the size_t variable to hold the number of bytes
 *                  written.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_WOULD_BLOCK if the operation would cause the process
 *        to block and no blocking has been set.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is bad.
 *      - VCTOOL_ERROR_FILE_QUOTA if this operation violates a user quota on
 *        disk space.
 *      - VCTOOL_ERROR_FILE_FAULT if this operation causes a memory fault.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if an attempt is made to write to a file
 *        that exceeds disk or user limits.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation is interrupted by a
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_INVALID_FLAGS if this operation violates flags set
 *        for this descriptor.
 *      - VCTOOL_ERROR_FILE_IO if a low-level I/O error occurs.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if there is no space left on this device.
 *      - VCTOOL_ERROR_FILE_ACCESS if an access / permission issue occurs.
 *      - VCTOOL_ERROR_FILE_BROKEN_PIPE if the remote end of this descriptor is
 *        disconnected.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error occurs.
 */
int file_write(file* f, int d, const void* buf, size_t max, size_t* wbytes);

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
    file* f, int d, off_t offset, file_lseek_whence whence, off_t* newoffset);

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
int file_fsync(file* f, int d);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_FILE_HEADER_GUARD*/

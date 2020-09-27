/**
 * \file file/file_init.c
 *
 * \brief Implementation of file_init.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <vctool/file.h>
#include <vpr/parameters.h>

/* forward decls. */
static void file_os_dispose(void*);
static int file_os_stat(file*, const char*, file_stat_st*);
static int file_os_open(file*, int*, const char*, int, mode_t);
static int file_os_close(file*, int);
static int file_os_read(file*, int, void*, size_t, size_t*);
static int file_os_write(file*, int, const void*, size_t, size_t*);
static int file_os_lseek(file*, int, off_t, file_lseek_whence, off_t*);
static int file_os_fsync(file*, int);

/**
 * \brief Initialize a file interface backed by the operating system.
 *
 * \param f         The file interface to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int file_init(file* f)
{
    MODEL_ASSERT(NULL != f);

    /* clear out structure. */
    memset(f, 0, sizeof(file));

    /* set dispose method. */
    f->hdr.dispose = &file_os_dispose;

    /* set file os methods. */
    f->file_stat_method = &file_os_stat;
    f->file_open_method = &file_os_open;
    f->file_close_method = &file_os_close;
    f->file_read_method = &file_os_read;
    f->file_write_method = &file_os_write;
    f->file_lseek_method = &file_os_lseek;
    f->file_fsync_method = &file_os_fsync;

    /* the file instance should now be valid. */
    MODEL_ASSERT(PROP_FILE_VALID(f));

    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a file instance.
 *
 * \param disp      file instance to dispose.
 */
static void file_os_dispose(void* disp)
{
    file* f = (file*)disp;

    /* only dispose of valid file instances. */
    MODEL_ASSERT(PROP_FILE_VALID(f));

    memset(f, 0, sizeof(file));
}

/**
 * \brief OS stat implementation.
 *
 * \param f         The file instance for this implementation.
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
static int file_os_stat(
    file* UNUSED(f), const char* path, file_stat_st* filestat)
{
    struct stat s;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(NULL != path);
    MODEL_ASSERT(NULL != filestat);

    /* attempt to call stat on the given path. */
    if (stat(path, &s) < 0)
    {
        switch (errno)
        {
            case EACCES:
                return VCTOOL_ERROR_FILE_ACCESS;
            case ELOOP:
                return VCTOOL_ERROR_FILE_LOOP;
            case ENAMETOOLONG:
                return VCTOOL_ERROR_FILE_NAME_TOO_LONG;
            case ENOENT:
                return VCTOOL_ERROR_FILE_NO_ENTRY;
            case ENOMEM:
                return VCTOOL_ERROR_FILE_KERNEL_MEMORY;
            case ENOTDIR:
                return VCTOOL_ERROR_FILE_NOT_DIRECTORY;
            case EOVERFLOW:
                return VCTOOL_ERROR_FILE_OVERFLOW;
            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    /* on success, populate the file stat struct based on the stat struct. */
    filestat->fst_mode = s.st_mode;
    filestat->fst_uid = s.st_uid;
    filestat->fst_gid = s.st_gid;
    filestat->fst_size = s.st_size;

    return VCTOOL_STATUS_SUCCESS;
}

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
static int file_os_open(
    file* UNUSED(f), int* d, const char* path, int flags, mode_t mode)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(NULL != d);
    MODEL_ASSERT(NULL != path);

    *d = open(path, flags, mode);
    if (*d < 0)
    {
        switch (errno)
        {
            case ETXTBSY: /* fall-through */
            case EPERM: /* fall-through */
            case EACCES:
                return VCTOOL_ERROR_FILE_ACCESS;
            case EDQUOT:
                return VCTOOL_ERROR_FILE_QUOTA;
            case EEXIST:
                return VCTOOL_ERROR_FILE_EXISTS;
            case EFAULT:
                return VCTOOL_ERROR_FILE_FAULT;
            case EINTR:
                return VCTOOL_ERROR_FILE_INTERRUPT;
            case EINVAL:
                return VCTOOL_ERROR_FILE_INVALID_FLAGS;
            case EISDIR:
                return VCTOOL_ERROR_FILE_IS_DIRECTORY;
            case ELOOP:
                return VCTOOL_ERROR_FILE_LOOP;
            case EMFILE:
                return VCTOOL_ERROR_FILE_TOO_MANY_FILES;
            case ENAMETOOLONG:
                return VCTOOL_ERROR_FILE_NAME_TOO_LONG;
            case ENFILE:
                return VCTOOL_ERROR_FILE_TOO_MANY_FILES;
            case ENXIO:  /* fall-through */
            case ENOENT: /* fall-through */
            case ENODEV:
                return VCTOOL_ERROR_FILE_NO_ENTRY;
            case ENOMEM:
                return VCTOOL_ERROR_FILE_KERNEL_MEMORY;
            case ENOSPC:
                return VCTOOL_ERROR_FILE_NO_SPACE;
            case ENOTDIR:
                return VCTOOL_ERROR_FILE_NOT_DIRECTORY;
            case EROFS: /* fall-through */
            case EOPNOTSUPP:
                return VCTOOL_ERROR_FILE_NOT_SUPPORTED;
            case EFBIG: /* fall-through */
            case EOVERFLOW:
                return VCTOOL_ERROR_FILE_OVERFLOW;
            case EWOULDBLOCK:
                return VCTOOL_ERROR_FILE_WOULD_BLOCK;
            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    return VCTOOL_STATUS_SUCCESS;
}

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
static int file_os_close(file* UNUSED(f), int d)
{
    if (close(d) < 0)
    {
        switch (errno)
        {
            case EBADF:
                return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
            case EINTR:
                return VCTOOL_ERROR_FILE_INTERRUPT;
            case EIO:
                return VCTOOL_ERROR_FILE_IO;
            case ENOSPC:
                return VCTOOL_ERROR_FILE_NO_SPACE;
            case EDQUOT:
                return VCTOOL_ERROR_FILE_QUOTA;
            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    return VCTOOL_STATUS_SUCCESS;
}

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
static int file_os_read(
    file* UNUSED(f), int d, void* buf, size_t max, size_t* rbytes)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != rbytes);

    /* attempt to read from this fd. */
    ssize_t retval = read(d, buf, max);
    if (retval < 0)
    {
        switch (errno)
        {
            case EWOULDBLOCK:
                return VCTOOL_ERROR_FILE_WOULD_BLOCK;
            case EBADF:
                return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
            case EFAULT:
                return VCTOOL_ERROR_FILE_FAULT;
            case EINTR:
                return VCTOOL_ERROR_FILE_INTERRUPT;
            case EINVAL:
                return VCTOOL_ERROR_FILE_INVALID_FLAGS;
            case EIO:
                return VCTOOL_ERROR_FILE_IO;
            case EISDIR:
                return VCTOOL_ERROR_FILE_IS_DIRECTORY;
            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    /* save the number of bytes read. */
    *rbytes = retval;

    return VCTOOL_STATUS_SUCCESS;
}

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
static int file_os_write(
    file* UNUSED(f), int d, const void* buf, size_t max, size_t* wbytes)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != wbytes);

    /* attempt to write to this fd. */
    ssize_t retval = write(d, buf, max);
    if (retval < 0)
    {
        switch (errno)
        {
            case EWOULDBLOCK:
                return VCTOOL_ERROR_FILE_WOULD_BLOCK;
            case EBADF:
                return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
            case EDQUOT:
                return VCTOOL_ERROR_FILE_QUOTA;
            case EFAULT:
                return VCTOOL_ERROR_FILE_FAULT;
            case EFBIG:
                return VCTOOL_ERROR_FILE_OVERFLOW;
            case EINTR:
                return VCTOOL_ERROR_FILE_INTERRUPT;
            case EINVAL:
                return VCTOOL_ERROR_FILE_INVALID_FLAGS;
            case EIO:
                return VCTOOL_ERROR_FILE_IO;
            case ENOSPC:
                return VCTOOL_ERROR_FILE_NO_SPACE;
            case EPERM:
                return VCTOOL_ERROR_FILE_ACCESS;
            case EPIPE:
                return VCTOOL_ERROR_FILE_BROKEN_PIPE;
            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    /* save the number of bytes written. */
    *wbytes = retval;

    return VCTOOL_STATUS_SUCCESS;
}

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
static int file_os_lseek(
    file* UNUSED(f), int d, off_t offset, file_lseek_whence whence,
    off_t* newoffset)
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

    /* decode whence. */
    int os_whence = 0;
    switch (whence)
    {
        case FILE_LSEEK_WHENCE_ABSOLUTE:
            os_whence = SEEK_SET;
            break;

        case FILE_LSEEK_WHENCE_CUR:
            os_whence = SEEK_CUR;
            break;

        case FILE_LSEEK_WHENCE_END:
            os_whence = SEEK_END;
            break;

        /* These are Linux extensions. */
#ifdef _GNU_SOURCE
        case FILE_LSEEK_WHENCE_DATA:
            os_whence = SEEK_DATA;
            break;

        case FILE_LSEEK_WHENCE_HOLE:
            os_whence = SEEK_HOLE;
            break;
#endif

        default:
            return VCTOOL_ERROR_FILE_INVALID;
    }

    /* attempt to perform the lseek operation. */
    off_t retval = lseek(d, offset, os_whence);
    if (retval < 0)
    {
        switch (errno)
        {
            case EBADF:
                return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;

            case EINVAL:
                return VCTOOL_ERROR_FILE_INVALID;

            case ENXIO:
                return VCTOOL_ERROR_FILE_BAD_ADDRESS;

            case EOVERFLOW:
                return VCTOOL_ERROR_FILE_OVERFLOW;

            case ESPIPE:
                return VCTOOL_ERROR_FILE_IS_PIPE;

            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    /* assign newoffset on success. */
    *newoffset = retval;

    return VCTOOL_STATUS_SUCCESS;
}

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
static int file_os_fsync(file* UNUSED(f), int d)
{
   /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);

    /* attempt to perform the lseek operation. */
    int retval = fsync(d);
    if (0 != retval)
    {
        switch (errno)
        {
            case EBADF:
                return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;

            case EIO:
                return VCTOOL_ERROR_FILE_IO;

            case ENOSPC:
                return VCTOOL_ERROR_FILE_NO_SPACE;

            case EROFS: /* fall-through */
            case EINVAL:
                return VCTOOL_ERROR_FILE_INVALID;

            case EDQUOT:
                return VCTOOL_ERROR_FILE_QUOTA;

            default:
                return VCTOOL_ERROR_FILE_UNKNOWN;
        }
    }

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

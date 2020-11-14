/**
 * \file include/vctool/backup.h
 *
 * \brief Backup file structure and serialization methods.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_BACKUP_HEADER_GUARD
# define VCTOOL_BACKUP_HEADER_GUARD

#include <stdint.h>
#include <vccrypt/suite.h>
#include <vctool/file.h>
#include <vpr/disposable.h>
#include <vpr/uuid.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/* forward decls. */
typedef struct backup_file_enc_header backup_file_enc_header;
typedef struct backup_record_header backup_record_header;
typedef struct backup_record_root backup_record_root;
typedef struct backup_record_accounting backup_record_accounting;
typedef struct backup_record_block backup_record_block;

/** \brief This macro performs the crypto padding operation. */
#define CRYPTO_PAD(x) \
    (((x) % 16 == 0) \
         ? (x) + 16 \
         : (x) + (16 - ((x) % 16)))

/**
 * \brief This is an enumeration of the record types supported by the blockchain
 * backup file.
 */
enum backup_record_type
{
    BACKUP_RECORD_TYPE_ROOT,
    BACKUP_RECORD_TYPE_ACCOUNTING,
    BACKUP_RECORD_TYPE_BLOCK,
};

/** \brief The current version is 0.1 */
#define BACKUP_FILE_ENC_HEADER_SERIALIZATION_VERSION 0x0000000010000000UL

/**
 * \brief At the beginning of the backup file is this encryption header.
 */
struct backup_file_enc_header
{
    /** \brief This record is disposable. */
    disposable_t hdr;

    /** \brief "MAGIC" for this file. "ENCVCBAK*/
    uint8_t file_magic[8];

    /** \brief The serialization version of this file. */
    uint64_t serialization_version;

    /** \brief The size of this record. */
    uint64_t record_size;

    /** \brief The number of rounds used in the PBKDRF for this key. */
    uint64_t rounds;

    /** \brief The salt for deriving the passphrase key. */
    uint8_t passphrase_salt[32];

    /** \brief The encrypted key in AES-2X-256-CBC. */
    uint8_t enc_key[48];

    /** \brief The MAC for this header, MACed with the encrypted key. */
    uint8_t file_header_mac[32];
};

/**
 * \brief The size of the backup_file_enc_header on disk.
 */
#define BACKUP_FILE_SIZE_FILE_ENC_HEADER \
    (   ( 8 * sizeof(uint8_t))      /* the magic. */ \
      +       sizeof(uint64_t)      /* the serialization version. */ \
      +       sizeof(uint64_t)      /* the total record size. */ \
      +       sizeof(uint64_t)      /* the number of rounds. */ \
      + (32 * sizeof(uint8_t))      /* the passphrase salt. */ \
      + (48 * sizeof(uint8_t))      /* the encrypted key. */ \
      + (32 * sizeof(uint8_t)))     /* the record mac. */

/**
 * \brief Backup file record header.
 */
struct backup_record_header
{
    /** \brief This record is disposable. */
    disposable_t hdr;

    /** \brief Record IV. */
    uint8_t iv[16];

    /** \brief The record type. */
    uint32_t type;

    /** \brief Reserved field. Must be all zeroes in this version. */
    uint32_t reserved;

    /** \brief The total record size. */
    uint64_t record_size;

    /** \brief The record mac. */
    uint8_t record_mac[32];
};

/**
 * \brief The raw size of the backup_record_header on disk.
 */
#define BACKUP_FILE_SIZE_RECORD_HEADER_RAW \
    (   (16 * sizeof(uint8_t))      /* the record IV. */ \
      +       sizeof(uint32_t)      /* the record type. */ \
      +       sizeof(uint32_t)      /* the reserved field. */ \
      +       sizeof(uint64_t)      /* the total record size. */ \
      + (32 * sizeof(uint8_t)))     /* the record MAC. */

/**
 * \brief Backup file root record.
 */
struct backup_record_root
{
    /** \brief This is a backup record. */
    backup_record_header hdr;

    /** \brief File format version. */
    uint64_t format_version;

    /** \brief Offset to the accounting record. */
    uint64_t offset_accounting_record;

    /** \brief Offset to the first backup block. */
    uint64_t offset_first_backup_block;

    /** \brief Offset to the last backup block. */
    uint64_t offset_last_backup_block;

    /** \brief Offset to the current end of file. */
    uint64_t offset_eof;
};

/**
 * \brief The raw size of the backup_record_root record on disk.
 */
#define BACKUP_FILE_SIZE_RECORD_ROOT_RAW \
    (   BACKUP_FILE_SIZE_RECORD_HEADER_RAW  /* this is a record. */ \
      +       sizeof(uint64_t)              /* the file format version. */ \
      +       sizeof(uint64_t)              /* accounting record offset. */ \
      +       sizeof(uint64_t)              /* first backup block offset. */ \
      +       sizeof(uint64_t)              /* last backup block offset. */ \
      +       sizeof(uint64_t))             /* end of file offset. */

/**
 * \brief The padded size of the backup_record_root record on disk.
 */
#define BACKUP_FILE_SIZE_RECORD_ROOT_PADDED \
    CRYPTO_PAD(BACKUP_FILE_SIZE_RECORD_ROOT_RAW)

/**
 * \brief Backup file record accounting. This record is held in write lock the
 * whole time the backup process is running.
 */
struct backup_record_accounting
{
    /** \brief This is a backup record. */
    backup_record_header hdr;

    /** \brief File creation date. */
    uint64_t date_creation;

    /** \brief File last update date. */
    uint64_t date_update;

    /** \brief Total number of blocks in the file. */
    uint64_t file_total_blocks;

    /** \brief Total number of blocks currently known upstream. */
    uint64_t upstream_total_blocks;

    /** \brief The root block id. */
    vpr_uuid root_block;

    /** \brief The first non-root block id. */
    vpr_uuid first_block;

    /** \brief The last block id. */
    vpr_uuid last_block;
};

/**
 * \brief The raw size of the backup_record_accounting record on disk.
 */
#define BACKUP_FILE_SIZE_RECORD_ACCOUNTING_RAW \
    (   BACKUP_FILE_SIZE_RECORD_HEADER_RAW  /* this is a record. */ \
      +       sizeof(uint64_t)              /* creation date of this file. */ \
      +       sizeof(uint64_t)              /* update date of this file. */ \
      +       sizeof(uint64_t)              /* file number of blocks. */ \
      +       sizeof(uint64_t)              /* upstream number of blocks. */ \
      + (16 * sizeof(uint8_t))              /* root block id. */ \
      + (16 * sizeof(uint8_t))              /* first block id. */ \
      + (16 * sizeof(uint8_t)))             /* last block id. */ \

/**
 * \brief The padded size of the backup_record_accounting record on disk.
 */
#define BACKUP_FILE_SIZE_RECORD_ACCOUNTING_PADDED \
    CRYPTO_PAD(BACKUP_FILE_SIZE_RECORD_ACCOUNTING_RAW)

/**
 * \brief A block in the blockchain backup.
 */
struct backup_record_block
{
    /** \brief This is a backup record. */
    backup_record_header hdr;

    /** \brief The block id. */
    vpr_uuid block_id;

    /** \brief The block height. */
    uint64_t block_height;

    /** \brief The block size. */
    uint64_t block_size;

    /** \brief The block proper. */
    vccrypt_buffer_t block_data;
};

/**
 * \brief The size of the backup_record_block header on disk.
 */
#define BACKUP_FILE_SIZE_RECORD_BLOCK_HEADER_RAW \
    (   BACKUP_FILE_SIZE_RECORD_HEADER_RAW  /* this is a record. */ \
      + (16 * sizeof(uint8_t))              /* block id. */ \
      +       sizeof(uint64_t)              /* block height. */ \
      +       sizeof(uint64_t))             /* block size. */ \

/**
 * \brief Write a backup file encryption header to a file instance.
 *
 * \param f                 The file instance to which this header is written.
 * \param desc              The file descriptor to which this header is written.
 * \param suite             The crypto suite to use for this operation.
 * \param passphrase        The passphrase to be used to decrypt this file.
 * \param rounds            The number of rounds to use to derive an encryption
 *                          key from the passphrase.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int backup_file_encryption_header_write(
    file* f, int desc, vccrypt_suite_options_t* suite,
    vccrypt_buffer_t* passphrase, uint64_t rounds);

/**
 * \brief Read a backup file encryption header from the given file instance.
 *
 * \param f                 The file instance from which the header is read.
 * \param desc              The file descriptor from which the header is read.
 * \param suite             The crypto suite to use for this operation.
 * \param passphrase        The passphrase to be used to decrypt this file.
 * \param header            Pointer to the header to be read by this operation.
 *                          On success, this header is owned by the caller and
 *                          must be disposed when no longer needed.
 * \param key               Pointer to an uninitialized buffer to be initialized
 *                          with the decrypted file key on success. On success,
 *                          this key buffer is owned by the caller and must be
 *                          disposed when no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int backup_file_encryption_header_read(
    file* f, int desc, vccrypt_suite_options_t* suite,
    vccrypt_buffer_t* passphrase, backup_file_enc_header* header,
    vccrypt_buffer_t* key);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_BACKUP_HEADER_GUARD*/

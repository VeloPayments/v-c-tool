/**
 * \file test/file/test_backup_structs.cpp
 *
 * \brief Unit tests for backup structures.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <minunit/minunit.h>
#include <vctool/backup.h>

/* start of the backup_structs test suite. */
TEST_SUITE(backup_structs);

/* Verify that each file size is a multiple of the AES block size. */
TEST(file_record_aes_block_size)
{
    int i = 0;

    TEST_ASSERT(i == 0);
    TEST_EXPECT(0 == (BACKUP_FILE_SIZE_RECORD_HEADER_RAW % 16));
    TEST_EXPECT(0 == (BACKUP_FILE_SIZE_RECORD_ROOT_PADDED % 16));
    TEST_EXPECT(0 == (BACKUP_FILE_SIZE_RECORD_ACCOUNTING_PADDED % 16));
    TEST_EXPECT(0 == (BACKUP_FILE_SIZE_RECORD_BLOCK_HEADER_RAW % 16));
}

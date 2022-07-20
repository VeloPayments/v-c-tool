/**
 * \file test/file/test_backup_file_encryption_header_write.cpp
 *
 * \brief Unit tests for backup_file_encryption_header_write.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <iomanip>
#include <iostream>
#include <minunit/minunit.h>
#include <string.h>
#include <vcblockchain/byteswap.h>
#include <vccrypt/mock_suite.h>
#include <vctool/backup.h>
#include <vector>
#include <vpr/allocator/malloc_allocator.h>

#include "../file/mock_file.h"

using namespace std;

/* start of the test suite. */
TEST_SUITE(backup_file_encryption_header_write);

/* Verify that parameters are null checked. */
TEST(parameter_checks)
{
    file f;
    vccrypt_suite_options_t suite;
    vccrypt_buffer_t passphrase;
    uint64_t rounds = 5000;
    int EXPECTED_DESC = 17;

    /* parameters are null checked. */
    TEST_EXPECT(
        VCTOOL_ERROR_BACKUP_BAD_PARAMETER ==
            backup_file_encryption_header_write(
                nullptr, EXPECTED_DESC, &suite, &passphrase, rounds));
    TEST_EXPECT(
        VCTOOL_ERROR_BACKUP_BAD_PARAMETER ==
            backup_file_encryption_header_write(
                &f, -1, &suite, &passphrase, rounds));
    TEST_EXPECT(
        VCTOOL_ERROR_BACKUP_BAD_PARAMETER ==
            backup_file_encryption_header_write(
                &f, EXPECTED_DESC, nullptr, &passphrase, rounds));
    TEST_EXPECT(
        VCTOOL_ERROR_BACKUP_BAD_PARAMETER ==
            backup_file_encryption_header_write(
                &f, EXPECTED_DESC, &suite, nullptr, rounds));
}

/* Verify that each expected method is called to build up the file header. */
TEST(happy_path)
{
    const char EXPECTED_PASSPHRASE[] = { 'T', 'e', 's', 't' };
    file f;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    vccrypt_buffer_t passphrase;
    uint64_t rounds = 5000;
    int EXPECTED_DESC = 17;
    uint8_t FAKE_DERIVED_KEY[32] = {
        0x20, 0x9e, 0x81, 0x23, 0xfb, 0xa6, 0x42, 0x68,
        0xb7, 0x39, 0x90, 0x59, 0xbd, 0xdc, 0x86, 0xab,
        0xa8, 0x64, 0xcb, 0x53, 0x76, 0x93, 0x4a, 0xe6,
        0x87, 0x14, 0x38, 0x62, 0xee, 0x7a, 0x83, 0x4c };
    uint8_t FAKE_MAC[32] = {
        0xa3, 0xe6, 0xe3, 0xba, 0x80, 0xa4, 0x47, 0x7d,
        0xa6, 0xd6, 0xe4, 0xbb, 0x87, 0x75, 0x79, 0xc8,
        0xf3, 0x63, 0xbf, 0xea, 0xa9, 0x81, 0x4c, 0x46,
        0xb2, 0xa3, 0xb5, 0x8d, 0xea, 0x76, 0x83, 0xd1 };

    /* register the mock crypto suite. */
    vccrypt_suite_register_mock();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create a passphrase buffer. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
            vccrypt_buffer_init(
                &passphrase, &alloc_opts, sizeof(EXPECTED_PASSPHRASE)));

    /* create the mock file. */
    bool file_write_called = false;
    uint8_t file_write_buf[BACKUP_FILE_SIZE_FILE_ENC_HEADER];
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f,
                /* stat. */
                [&](file*, const char*, file_stat_st*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* open. */
                [&](file*, int*, const char*, int, mode_t) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* close. */
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* read. */
                [&](file*, int, void*, size_t, size_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                /* write. */
                [&](
                    file* pf, int desc, const void* rec, size_t size,
                    size_t* written) -> int {
                        if (
                            size == BACKUP_FILE_SIZE_FILE_ENC_HEADER
                         && pf == &f && desc == EXPECTED_DESC)
                        {
                            file_write_called = true;
                            memcpy(file_write_buf, rec, size);
                            *written = size;

                            return VCTOOL_STATUS_SUCCESS;
                        }
                        else
                        {
                            return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                        }
                },
                /* lseek. */
                [&](file*, int, off_t, file_lseek_whence, off_t*) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                },
                [&](file*, int) -> int {
                    return VCTOOL_ERROR_FILE_BAD_DESCRIPTOR;
                }));

    /* create a mock crypto suite. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_options_init(
            &suite, &alloc_opts));

    /* add a mock for the prng init method. */
    bool prng_init_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_prng_init(
            &suite,
            [&](vccrypt_prng_options_t*, vccrypt_prng_context_t*) -> int {
                prng_init_called = true;
                return VCCRYPT_STATUS_SUCCESS;
            }));

    /* add a mock for the prng dispose method. */
    bool prng_dispose_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_prng_dispose(
            &suite,
            [&](vccrypt_prng_options_t*, vccrypt_prng_context_t*) -> void {
                prng_dispose_called = true;
            }));

    /* add a mock for the prng read method. */
    vector<size_t> prng_read_vec;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_prng_read(
            &suite,
            [&](vccrypt_prng_context_t*, uint8_t*, size_t size) -> int {
                prng_read_vec.push_back(size);
                return VCCRYPT_STATUS_SUCCESS;
            }));

    /* add a mock for key derivation init. */
    bool key_derivation_init_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_key_derivation_init(
            &suite,
            [&](
                vccrypt_key_derivation_context_t*,
                vccrypt_key_derivation_options_t*) -> int {
                    key_derivation_init_called = true;
                    return VCCRYPT_STATUS_SUCCESS;
            }));

    /* add a mock for key derivation dispose. */
    bool key_derivation_dispose_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_key_derivation_dispose(
            &suite,
            [&](
                vccrypt_key_derivation_context_t*,
                vccrypt_key_derivation_options_t*) {
                    key_derivation_dispose_called = true;
            }));

    /* add a mock for the key derivation derive key method. */
    bool key_derivation_derive_key_called = false;
    const vccrypt_buffer_t* derive_key_pass_buffer = nullptr;
    unsigned int derive_key_rounds = 0;
    bool derive_key_buffer_set = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_key_derivation_derive_key(
            &suite,
            [&](
                vccrypt_buffer_t* buffer,
                vccrypt_key_derivation_context_t*,
                const vccrypt_buffer_t* pass,
                const vccrypt_buffer_t*,
                unsigned int rounds) -> int {
                    key_derivation_derive_key_called = true;
                    derive_key_pass_buffer = pass;
                    derive_key_rounds = rounds;
                    if (NULL != buffer && buffer->size == 32)
                    {
                        memcpy(buffer->data, FAKE_DERIVED_KEY, 32);
                        derive_key_buffer_set = true;
                    }

                    return VCTOOL_STATUS_SUCCESS;
            }));

    /* add a mock for the block cipher init method. */
    bool block_init_called = false;
    bool block_init_called_with_correct_key = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_block_init(
            &suite,
            [&](
                vccrypt_block_options_t*, vccrypt_block_context_t*,
                const vccrypt_buffer_t* key, bool enc) -> int {
                    if (enc)
                        block_init_called = true;
                    if (32 == key->size
                     && !memcmp(key->data, FAKE_DERIVED_KEY, 32))
                        block_init_called_with_correct_key = true;

                return VCCRYPT_STATUS_SUCCESS;
            }));

    /* mock the block dispose method. */
    bool block_dispose_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_block_dispose(
            &suite,
            [&](
                vccrypt_block_options_t*, vccrypt_block_context_t*) {
                    block_dispose_called = true;
            }));

    /* mock the block encrypt function. */
    vector<const uint8_t*> block_encrypt_ivs;
    vector<const uint8_t*> block_encrypt_inputs;
    vector<uint8_t*> block_encrypt_outputs;
    int block_encrypt_call_count = 0;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_block_encrypt(
            &suite,
            [&](
                vccrypt_block_context_t*, const void* iv,
                const void* input, void* output) -> int {
                    ++block_encrypt_call_count;
                    block_encrypt_ivs.push_back((const uint8_t*)iv);
                    block_encrypt_inputs.push_back((const uint8_t*)input);
                    block_encrypt_outputs.push_back((uint8_t*)output);
                    return VCCRYPT_STATUS_SUCCESS;
            }));

    /* add a mock for the mac init method. */
    bool mac_init_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_short_mac_init(
            &suite,
            [&](
                vccrypt_mac_options_t*, vccrypt_mac_context_t*,
                const vccrypt_buffer_t*) -> int {
                    mac_init_called = true;
                    return VCCRYPT_STATUS_SUCCESS;
            }));

    /* add a mock for the mac dispose method. */
    bool mac_dispose_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_short_mac_dispose(
            &suite,
            [&](vccrypt_mac_options_t*, vccrypt_mac_context_t*) {
                mac_dispose_called = true;
        }));

    /* add a mock for the mac digest method. */
    bool mac_digest_called = false;
    uint8_t hdr[BACKUP_FILE_SIZE_FILE_ENC_HEADER];
    memset(&hdr, 0, BACKUP_FILE_SIZE_FILE_ENC_HEADER);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_short_mac_digest(
            &suite,
            [&](
                vccrypt_mac_context_t*, const uint8_t* data,
                size_t size) -> int {
                    mac_digest_called = true;
                    memcpy(hdr, data, size);

                    return VCCRYPT_STATUS_SUCCESS;
            }));

    /* add a mock for the mac finalize method. */
    bool mac_finalize_called = false;
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS ==
        vccrypt_mock_suite_add_mock_short_mac_finalize(
            &suite,
            [&](vccrypt_mac_context_t*, vccrypt_buffer_t* digest) -> int {
                if (NULL != digest && NULL != digest->data
                 && 32 == digest->size)
                {
                    mac_finalize_called = true;
                    memcpy(digest->data, FAKE_MAC, 32);
                    return VCCRYPT_STATUS_SUCCESS;
                }
                else
                {
                    return VCCRYPT_ERROR_MAC_FINALIZE_INVALID_ARG;
                }
            }));

    /* calling backup_file_encryption_header_write should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
        backup_file_encryption_header_write(
            &f, EXPECTED_DESC, &suite, &passphrase, rounds));

    /* The first thing that this method should do is create a prng instance. */
    TEST_EXPECT(prng_init_called);

    /* Eventually, this method should be cleaned up... */
    TEST_EXPECT(prng_dispose_called);

    /* The PRNG should have been read 3 times.. */
    TEST_ASSERT(3 == prng_read_vec.size());
    /* first, the passphrase salt. */
    TEST_EXPECT(32 == prng_read_vec[0]);
    /* then, the encryption key iv. */
    TEST_EXPECT(16 == prng_read_vec[1]);
    /* finally, the short-term encryption key. */
    TEST_EXPECT(32 == prng_read_vec[2]);

    /* the key derivation instance was initialized. */
    TEST_EXPECT(key_derivation_init_called);

    /* the key derivation dispose method is eventually called. */
    TEST_EXPECT(key_derivation_dispose_called);

    /* the key derive method was called. */
    TEST_EXPECT(key_derivation_derive_key_called);
    /* the number of rounds was collect. */
    TEST_EXPECT(rounds == derive_key_rounds);
    /* the password buffer was correct. */
    TEST_EXPECT(&passphrase == derive_key_pass_buffer);
    /* the buffer was set. */
    TEST_EXPECT(derive_key_buffer_set);

    /* block init was called. */
    TEST_EXPECT(block_init_called);
    /* block init was called with the correct key. */
    TEST_EXPECT(block_init_called_with_correct_key);

    /* Eventually, block_dispose was called. */
    TEST_EXPECT(block_dispose_called);

    /* block encrypt was called twice. */
    TEST_ASSERT(2 == block_encrypt_call_count);
    /* the IVs were different. */
    TEST_EXPECT(block_encrypt_ivs[0] != block_encrypt_ivs[1]);
    /* the second IV was the first output. */
    TEST_EXPECT(block_encrypt_ivs[1] == block_encrypt_outputs[0]);
    /* the outputs differ by a block (16 bytes). */
    TEST_EXPECT(block_encrypt_outputs[1] - block_encrypt_outputs[0] == 16);

    /* mac init called. */
    TEST_EXPECT(mac_init_called);
    /* mac dispose called. */
    TEST_EXPECT(mac_dispose_called);

    /* mac digest called. */
    TEST_EXPECT(mac_digest_called);
    /* the first 8 bytes are set correctly. */
    TEST_EXPECT(!memcmp(hdr, "ENCVCBAK", 8));
    /* the next 8 bytes are set to the serialization version. */
    uint64_t net_serial_version =
        htonll(BACKUP_FILE_ENC_HEADER_SERIALIZATION_VERSION);
    TEST_EXPECT(!memcmp(hdr + 8, &net_serial_version, 8));
    /* the next 8 bytes are set to the record size. */
    uint64_t net_record_size = htonll(BACKUP_FILE_SIZE_FILE_ENC_HEADER);
    TEST_EXPECT(!memcmp(hdr + 16, &net_record_size, 8));
    /* the next 8 bytes are set to the number of rounds. */
    uint64_t net_rounds = htonll(rounds);
    TEST_EXPECT(!memcmp(hdr + 24, &net_rounds, 8));

    /* mac finalize called. */
    TEST_EXPECT(mac_finalize_called);

    /* file write called. */
    TEST_EXPECT(file_write_called);
    /* the header matches the computed header. */
    TEST_EXPECT(
        !memcmp(
            file_write_buf, hdr, BACKUP_FILE_SIZE_FILE_ENC_HEADER - 32));
    /* the mac matches. */
    TEST_EXPECT(
        !memcmp(
            file_write_buf + BACKUP_FILE_SIZE_FILE_ENC_HEADER - 32, FAKE_MAC,
            32));

    /* clean up. */
    dispose((disposable_t*)&suite);
    dispose((disposable_t*)&f);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * \file backup/backup_file_encryption_header_write.c
 *
 * \brief Write a file encryption header.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vcblockchain/byteswap.h>
#include <vctool/backup.h>

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
    vccrypt_buffer_t* passphrase, uint64_t rounds)
{
    int retval;
    vccrypt_prng_context_t prng;
    vccrypt_buffer_t salt_buffer;
    vccrypt_buffer_t iv_buffer;
    vccrypt_buffer_t lt_key_buffer;
    vccrypt_buffer_t st_key_buffer;
    vccrypt_buffer_t encrypted_key_buffer;
    vccrypt_buffer_t record_buffer;
    vccrypt_buffer_t mac_buffer;
    vccrypt_key_derivation_context_t key;
    vccrypt_block_context_t block;
    vccrypt_mac_context_t mac;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != f);
    MODEL_ASSERT(desc >= 0);
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != passphrase);

    /* runtime parameter checks. */
    if (NULL == f || desc < 0 || NULL == suite || NULL == passphrase)
    {
        retval = VCTOOL_ERROR_BACKUP_BAD_PARAMETER;
        goto done;
    }

    /* create a PRNG instance. */
    retval = vccrypt_suite_prng_init(suite, &prng);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create an IV buffer. */
    retval =
        vccrypt_buffer_init(
            &iv_buffer, suite->alloc_opts, 16);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* create a salt buffer. */
    retval =
        vccrypt_buffer_init(
            &salt_buffer, suite->alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_iv_buffer;
    }

    /* create a long-term key buffer. */
    retval =
        vccrypt_buffer_init(
            &lt_key_buffer, suite->alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_salt_buffer;
    }

    /* create a short-term key buffer. */
    retval =
        vccrypt_buffer_init(
            &st_key_buffer, suite->alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_lt_key_buffer;
    }

    /* create an encrypted key buffer. */
    retval =
        vccrypt_buffer_init(
            &encrypted_key_buffer, suite->alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_st_key_buffer;
    }

    /* create the record buffer. */
    retval =
        vccrypt_buffer_init(
            &record_buffer, suite->alloc_opts,
            BACKUP_FILE_SIZE_FILE_ENC_HEADER);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_key_buffer;
    }

    /* create the mac buffer. */
    retval =
        vccrypt_suite_buffer_init_for_mac_authentication_code(
            suite, &mac_buffer, true);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_record_buffer;
    }

    /* read the passphrase salt. */
    retval =
        vccrypt_prng_read(&prng, &salt_buffer, salt_buffer.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* read the encryption key iv. */
    retval =
        vccrypt_prng_read(&prng, &iv_buffer, iv_buffer.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* read the short-term encryption key. */
    retval =
        vccrypt_prng_read(&prng, &st_key_buffer, st_key_buffer.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* create the key derivation instance. */
    retval =
        vccrypt_suite_key_derivation_init(&key, suite);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* derive the long-term key. */
    retval =
        vccrypt_key_derivation_derive_key(
            &lt_key_buffer, &key, passphrase, &salt_buffer, rounds);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_derivation;
    }

    /* create the block cipher instance using the lt key. */
    retval =
        vccrypt_suite_block_init(
            suite, &block, &lt_key_buffer, true);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_derivation;
    }

    /* use it to encrypt the short-term key (first block). */
    const uint8_t* kbuf = (const uint8_t*)st_key_buffer.data;
    uint8_t* outbuf = (uint8_t*)encrypted_key_buffer.data;
    retval = vccrypt_block_encrypt(&block, iv_buffer.data, kbuf, outbuf);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_block;
    }

    /* use it to encrypt the short-term key (second block). */
    retval = vccrypt_block_encrypt(&block, outbuf, kbuf + 16, outbuf + 16);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_block;
    }

    /* create a mac instance. */
    retval = vccrypt_suite_mac_short_init(suite, &mac, &st_key_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_block;
    }

    /* create a convenience pointer to the record buffer. */
    uint8_t* buf = (uint8_t*)record_buffer.data;

    /* write the file magic to the header. */
    memcpy(buf, "ENCVCBAK", 8); buf += 8;

    /* write the rounds to the header. */
    uint64_t net_rounds = htonll(rounds);
    memcpy(buf, &net_rounds, sizeof(net_rounds)); buf += sizeof(net_rounds);

    /* write the passphrase salt to the header. */
    memcpy(buf, salt_buffer.data, salt_buffer.size); buf += salt_buffer.size;

    /* write the encrypted key iv to the header. */
    memcpy(buf, iv_buffer.data, iv_buffer.size); buf += iv_buffer.size;

    /* write the encrypted key to the header. */
    memcpy(buf, encrypted_key_buffer.data, encrypted_key_buffer.size);
    buf += encrypted_key_buffer.size;

    /* mac the record buffer. */
    retval =
        vccrypt_mac_digest(
            &mac, record_buffer.data, record_buffer.size - 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* finalize the mac. */
    retval =
        vccrypt_mac_finalize(&mac, &mac_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* copy the mac to the record buffer. */
    memcpy(buf, mac_buffer.data, mac_buffer.size);

    /* write the record. */
    size_t wrote_size;
    retval =
        file_write(
            f, desc, record_buffer.data, record_buffer.size, &wrote_size);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* verify that we wrote all data. */
    if (wrote_size != record_buffer.size)
    {
        retval = VCTOOL_ERROR_BACKUP_TRUNCATED_RECORD;
        goto cleanup_mac;
    }

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;
    goto cleanup_mac;

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_block:
    dispose((disposable_t*)&block);

cleanup_key_derivation:
    dispose((disposable_t*)&key);

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

cleanup_record_buffer:
    dispose((disposable_t*)&record_buffer);

cleanup_encrypted_key_buffer:
    dispose((disposable_t*)&encrypted_key_buffer);

cleanup_st_key_buffer:
    dispose((disposable_t*)&st_key_buffer);

cleanup_lt_key_buffer:
    dispose((disposable_t*)&lt_key_buffer);

cleanup_salt_buffer:
    dispose((disposable_t*)&salt_buffer);

cleanup_iv_buffer:
    dispose((disposable_t*)&iv_buffer);

cleanup_prng:
    dispose((disposable_t*)&prng);

done:
    return retval;
}

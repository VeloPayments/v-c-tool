/**
 * \file command/endorse/endorse_build_output_file.c
 *
 * \brief Build the endorsed output file, given a working set, an input file,
 * and the endorser private certificate.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_uuid;

static inline size_t field_size(size_t value_size)
{
    /* a field has a type and size and a value. */
    return 2 * sizeof(uint16_t) + value_size;
}

static inline size_t uuid_field_size()
{
    return field_size(sizeof(rcpr_uuid));
}

static inline size_t signature_field_size(vccrypt_suite_options_t* suite)
{
    return field_size(suite->sign_opts.signature_size);
}

static inline size_t endorse_field_size()
{
    /* an endorse field has three UUIDs in it. */
    return field_size(3 * 16);
}

/**
 * \brief Build the output file given the output filename, the key certificate,
 * the working set, and the input file certificate.
 *
 * \param output_filename   The name of the output file.
 * \param opts              The commandline options for this operation.
 * \param key_cert          The endorser's key certificate.
 * \param set               The working set.
 * \param input_cert        The public key input certificate.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_build_output_file(
    const char* output_filename, commandline_opts* opts,
    const vccrypt_buffer_t* key_cert, RCPR_SYM(rbtree)* set,
    const vccrypt_buffer_t* input_cert)
{
    status retval;
    rcpr_uuid endorser_id;
    rcpr_uuid pub_id;
    vccrypt_buffer_t endorser_private_key;
    vccert_builder_options_t builder_opts;
    vccert_builder_context_t builder;
    int fd;

    /* get the endorser id and private signing key. */
    retval =
        endorse_get_endorser_details(
            &endorser_id, &endorser_private_key, opts, key_cert);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* get the number of entries in the working set. */
    size_t set_entries = rbtree_count(set);

    /* get the size of the input certificate. */
    size_t input_cert_size = input_cert->size;

    /* compute the size of the output certificate. */
    size_t output_cert_size =
        input_cert_size
      + (set_entries * endorse_field_size())
      + uuid_field_size()
      + signature_field_size(opts->suite);

    /* create a builder options instance. */
    retval =
        vccert_builder_options_init(
            &builder_opts, opts->suite->alloc_opts, opts->suite);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_private_key;
    }

    /* create a builder for the output certificate. */
    retval = vccert_builder_init(&builder_opts, &builder, output_cert_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_builder_opts;
    }

    /* write the public certificate fields to the builder. */
    retval =
        endorse_emit_public_certificate_fields(
            &builder, opts, &pub_id, input_cert);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* write the working set to the builder. */
    retval = endorse_emit_working_set(&builder, &pub_id, set);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* sign the certificate with the endorser id and private key. */
    retval =
        vccert_builder_sign(&builder, endorser_id.data, &endorser_private_key);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* emit the certificate from the builder. */
    const uint8_t* cert_data;
    size_t cert_size;
    cert_data = vccert_builder_emit(&builder, &cert_size);

    /* open the output file. */
    retval =
        file_open(
            opts->file, &fd, output_filename, O_CREAT | O_EXCL | O_WRONLY,
            S_IRUSR);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error opening output file %s.\n", output_filename);
        goto cleanup_builder;
    }

    /* write this cert to the output file. */
    size_t wrote_size;
    retval = file_write(opts->file, fd, cert_data, cert_size, &wrote_size);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error writing to output file.\n");
        goto cleanup_fd;
    }
    else if (wrote_size != cert_size)
    {
        fprintf(stderr, "Error: file truncated.\n");
        goto cleanup_fd;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_fd;

cleanup_fd:
    file_close(opts->file, fd);

cleanup_builder:
    dispose((disposable_t*)&builder);

cleanup_builder_opts:
    dispose((disposable_t*)&builder_opts);

cleanup_private_key:
    dispose((disposable_t*)&endorser_private_key);

done:
    return retval;
}

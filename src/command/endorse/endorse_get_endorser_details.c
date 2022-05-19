/**
 * \file command/endorse/endorse_get_endorser_details.c
 *
 * \brief Get the endorser id and private signing key.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <vccert/fields.h>
#include <vpr/parameters.h>

#include "endorse_internal.h"

/* forward decls. */
static bool dummy_txn_resolver(
    void*, void*, const uint8_t*, const uint8_t*, vccrypt_buffer_t*, bool*);
static int32_t dummy_artifact_state_resolver(
    void*, void*, const uint8_t*, vccrypt_buffer_t*);
static int dummy_contract_resolver(
    void*, void*, const uint8_t*, const uint8_t*, vccert_contract_closure_t*);
static bool dummy_key_resolver(
    void*, void*, uint64_t, const uint8_t*, vccrypt_buffer_t*,
    vccrypt_buffer_t*);

/**
 * \brief Get the endorser id and private signing key.
 *
 * \param endorser_id       Pointer to be populated with the endorser id.
 * \param private_key       Pointer to an uninitialized vccrypt buffer to be
 *                          initialized with the endorser private key.
 * \param opts              The command-line options for this operation.
 * \param key_cert          The endorser's private key certificate.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_get_endorser_details(
    RCPR_SYM(rcpr_uuid)* endorser_id, vccrypt_buffer_t* private_key,
    commandline_opts* opts, const vccrypt_buffer_t* key_cert)
{
    status retval;
    vccert_parser_options_t parser_options;
    vccert_parser_context_t parser;

    /* create parser options. */
    retval =
        vccert_parser_options_init(
            &parser_options, opts->suite->alloc_opts, opts->suite,
            &dummy_txn_resolver, &dummy_artifact_state_resolver,
            &dummy_contract_resolver, &dummy_key_resolver, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create a parser for the endorser certificate. */
    retval =
        vccert_parser_init(
            &parser_options, &parser, key_cert->data, key_cert->size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser_options;
    }

    /* get the endorser id. */
    const uint8_t* endorser_id_value = NULL;
    size_t endorser_id_size = 0U;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_ARTIFACT_ID, &endorser_id_value,
            &endorser_id_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* verify the endorser id. */
    size_t expected_uuid_size = 16;
    if (endorser_id_size != expected_uuid_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
        goto cleanup_parser;
    }

    /* copy the id. */
    memcpy(endorser_id, endorser_id_value, endorser_id_size);

    /* get the private signing key. */
    const uint8_t* private_signing_key_value = NULL;
    size_t private_signing_key_size = 0U;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PRIVATE_SIGNING_KEY,
            &private_signing_key_value, &private_signing_key_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* verify the private signing key size. */
    size_t expected_private_signing_key_size =
        opts->suite->sign_opts.private_key_size;
    if (private_signing_key_size != expected_private_signing_key_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
        goto cleanup_parser;
    }

    /* create a private signing key buffer. */
    retval =
        vccrypt_buffer_init(
            private_key, opts->suite->alloc_opts, private_signing_key_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* copy the private signing key. */
    memcpy(
        private_key->data, private_signing_key_value, private_signing_key_size);

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_parser;

cleanup_parser:
    dispose((disposable_t*)&parser);

cleanup_parser_options:
    dispose((disposable_t*)&parser_options);

done:
    return retval;
}

/**
 * \brief Dummy transaction resolver for parser options.
 */
static bool dummy_txn_resolver(
    void* UNUSED(a), void* UNUSED(b), const uint8_t* UNUSED(c),
    const uint8_t* UNUSED(d), vccrypt_buffer_t* UNUSED(e), bool* UNUSED(f))
{
    return false;
}

/**
 * \brief Dummy artifact state resolver for parser options.
 */
static int32_t dummy_artifact_state_resolver(
    void* UNUSED(a), void* UNUSED(b), const uint8_t* UNUSED(c),
    vccrypt_buffer_t* UNUSED(d))
{
    return -1;
}

/**
 * \brief Dummy contract resolver for parser options.
 */
static int dummy_contract_resolver(
    void* UNUSED(a), void* UNUSED(b), const uint8_t* UNUSED(c),
    const uint8_t* UNUSED(d), vccert_contract_closure_t* UNUSED(e))
{
    return -1;
}

/**
 * \brief Dummy key resolver for parser options.
 */
static bool dummy_key_resolver(
    void* UNUSED(a), void* UNUSED(b), uint64_t UNUSED(c),
    const uint8_t* UNUSED(d), vccrypt_buffer_t* UNUSED(e),
    vccrypt_buffer_t* UNUSED(f))
{
    return false;
}

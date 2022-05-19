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
 * \brief Write the public certificate fields to the builder.
 *
 * \param builder           The builder to which the fields are written.
 * \param opts              The command-line options for this operation.
 * \param pub_id            Pointer to UUID buffer to receive the public entity
 *                          id of this entity.
 * \param public_cert       The public certificate as a vccrypt buffer.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_emit_public_certificate_fields(
    vccert_builder_context_t* builder, commandline_opts* opts,
    RCPR_SYM(rcpr_uuid)* pub_id, const vccrypt_buffer_t* public_cert)
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

    /* create a parser for the public entity certificate. */
    retval =
        vccert_parser_init(
            &parser_options, &parser, public_cert->data, public_cert->size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser_options;
    }

    /* get the first field in the certificate. */
    uint16_t field_id;
    const uint8_t* value;
    size_t size;
    retval = vccert_parser_field_first(&parser, &field_id, &value, &size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* loop through all fields. */
    while (STATUS_SUCCESS == retval)
    {
        /* write the field to the builder. */
        retval =
            vccert_builder_add_short_buffer(builder, field_id, value, size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_parser;
        }

        /* if this field is the entity id, copy it to the pub id. */
        if (VCCERT_FIELD_TYPE_ARTIFACT_ID == field_id)
        {
            /* verify it's the right size. */
            if (size != 16)
            {
                retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
                goto cleanup_parser;
            }

            /* copy the field. */
            memcpy(pub_id, value, size);
        }

        /* get the next field. */
        retval = vccert_parser_field_next(&parser, &field_id, &value, &size);
    }

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

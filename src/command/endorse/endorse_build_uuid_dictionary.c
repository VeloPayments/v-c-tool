/**
 * \file command/endorse/endorse_build_uuid_dictionary.c
 *
 * \brief Build a map of key to UUID using the command-line options.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/**
 * \brief Build a map of key to UUID using the command-line options.
 *
 * \param dict              Receive a pointer to the dictionary on success.
 * \param alloc             The allocator to use for this operation.
 * \param opts              The command-line options to use.
 * \param root              The root command config.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_build_uuid_dictionary(
    RCPR_SYM(rbtree)** dict, RCPR_SYM(allocator)* alloc, commandline_opts* opts,
    const root_command* root)
{
    status retval, release_retval;
    rbtree* tmp;
    rbtree_node* kvp_nil;
    rbtree_node* kvp_x;
    root_dict_kvp* kvp_val;
    rcpr_uuid entity_id;
    certfile* pubkey_file;

    /* attempt to create an rbtree instance. */
    retval =
        rbtree_create(
            &tmp, alloc, &endorse_uuid_dictionary_compare,
            &endorse_uuid_dictionary_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* get the nil node for the root kvp dictionary. */
    kvp_nil = rbtree_nil_node((rbtree*)root->dict);

    /* get the root node for the root kvp dictionary. */
    kvp_x = rbtree_root_node((rbtree*)root->dict);

    /* starting at the minimum node, iterate through this dictionary. */
    kvp_x = rbtree_minimum_node((rbtree*)root->dict, kvp_x);
    for (
        ;
        kvp_nil != kvp_x;
        kvp_x = rbtree_successor_node((rbtree*)root->dict, kvp_x))
    {
        /* get the node value. */
        kvp_val = (root_dict_kvp*)rbtree_node_value((rbtree*)root->dict, kvp_x);

        /* get the public key file. */
        retval =
            endorse_get_pubkey_file(&pubkey_file, opts, root->alloc,
            kvp_val->value);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_tmp;
        }

        /* get the entity id from this file. */
        retval =
            endorse_get_pubkey_id(
                &entity_id, pubkey_file, opts, root->alloc, root);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_pubkey_file;
        }

        /* add a dict entry to our dictionary. */
        retval =
            endorse_uuid_dictionary_add(tmp, alloc, kvp_val->key, &entity_id);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_pubkey_file;
        }

        /* clean up pubkey file. */
        retval = resource_release(&pubkey_file->hdr);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_tmp;
        }
    }

    /* success. The caller now owns the dict. */
    *dict = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_pubkey_file:
    release_retval = resource_release(&pubkey_file->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_tmp:
    release_retval = resource_release(rbtree_resource_handle(tmp));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

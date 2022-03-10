/**
 * \file command/root/root_command_init.c
 *
 * \brief Initialize the root command.
 *
 * \copyright 2020-2022 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <rcpr/compare.h>
#include <string.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>

RCPR_IMPORT_compare;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/* forward decls. */
static void dispose_root_command(void* disp);
static const void* root_dict_key(
    void* context, const resource* r);
static rcpr_comparison_result root_dict_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Initialize a root command structure.
 *
 * \param root          The root command structure to initialize.
 * \param alloc         The RCPR allocator to use.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int root_command_init(root_command* root, RCPR_SYM(allocator)* alloc)
{
    status retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != root);

    /* clear the root structure. */
    memset(root, 0, sizeof(root_command));

    /* set root command values. */
    root->hdr.hdr.dispose = &dispose_root_command;
    root->key_derivation_rounds = ROOT_COMMAND_DEFAULT_KEY_DERIVATION_ROUNDS;
    root->alloc = alloc;

    /* create the root dict rbtree. */
    retval =
        rbtree_create(
            &root->dict, alloc, &root_dict_compare, &root_dict_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_root;
    }

    /* create the root permissions list. */
    retval = slist_create(&root->permissions, alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_root;
    }

    /* success. */
    goto done;

cleanup_root:
    dispose((disposable_t*)&root);

done:
    return retval;
}

/**
 * \brief Dispose the root command structure.
 *
 * \param disp          The root command structure to dispose.
 */
static void dispose_root_command(void* disp)
{
    root_command* root = (root_command*)disp;

    /* if the input filename is set, then free it. */
    if (NULL != root->input_filename)
    {
        free(root->input_filename);
    }

    /* if the output filename is set, then free it. */
    if (NULL != root->output_filename)
    {
        free(root->output_filename);
    }

    /* if the key filename is set, then free it. */
    if (NULL != root->key_filename)
    {
        free(root->key_filename);
    }

    /* if the endorse config filename is set, then free it. */
    if (NULL != root->endorse_config_filename)
    {
        free(root->endorse_config_filename);
    }

    /* if the dict was created, then release it. */
    if (NULL != root->dict)
    {
        (void)resource_release(rbtree_resource_handle(root->dict));
    }

    /* if the permissions list was created, then release it. */
    if (NULL != root->permissions)
    {
        (void)resource_release(slist_resource_handle(root->permissions));
    }
}

/**
 * \brief Compare two keys in the command root dictionary.
 *
 * \param context       Unused.
 * \param lhs           The left-hand side to compare.
 * \param rhs           The right-hand side to compare.
 *
 * \returns an integer value representing the comparison result.
 *      - RCPR_COMPARE_LT if \p lhs &lt; \p rhs.
 *      - RCPR_COMPARE_EQ if \p lhs == \p rhs.
 *      - RCPR_COMPARE_GT if \p lhs &gt; \p rhs.
 */
static rcpr_comparison_result root_dict_compare(
    void* context, const void* lhs, const void* rhs)
{
    (void)context;
    const char* l = (const char*) lhs;
    const char* r = (const char*) rhs;

    /* do a string comparison of both keys. */
    int val = strcmp(l, r);

    /* decode the result. */
    if (val < 0)
    {
        return RCPR_COMPARE_LT;
    }
    else if (val > 0)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}

/**
 * \brief Given a dictionary key-value pair, return the key.
 *
 * \param context       Unused.
 * \param r             The key-value pair resource.
 *
 * \returns the key for this resource.
 */
static const void* root_dict_key(
    void* context, const resource* r)
{
    (void)context;
    const root_dict_kvp* kvp = (const root_dict_kvp*)r;

    return (const void*)kvp->key;
}

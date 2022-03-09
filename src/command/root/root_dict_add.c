/**
 * \file command/root/root_dict_add.c
 *
 * \brief Add a key-value pair to the root command dictionary.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <rcpr/allocator.h>
#include <string.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/* forward decls. */
static status root_dict_split(char** key, char** value, const char* kvp);
static status root_dict_kvp_release(resource* r);

/**
 * \brief Add a key-value pair in the form of a string of "key=value" to the
 * root dictionary.
 *
 * \param root          The command-line root command.
 * \param kvp           The key-value pair to add.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int root_dict_add(root_command* root, const char* kvp)
{
    status retval, release_retval;
    char* key = NULL;
    char* value = NULL;
    root_dict_kvp* entry = NULL;

    /* attempt to split the kvp into a key and value. */
    retval = root_dict_split(&key, &value, kvp);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* allocate memory for the entry. */
    retval =
        rcpr_allocator_allocate(root->alloc, (void**)&entry, sizeof(*entry));
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_key_value;
    }

    /* clear this structure. */
    memset(entry, 0, sizeof(*entry));

    /* initialize this as a resource. */
    resource_init(&entry->hdr, &root_dict_kvp_release);

    /* set values. */
    entry->alloc = root->alloc;
    entry->key = key;
    entry->value = value;

    /* key and value are now owned by the entry instance. */
    key = value = NULL;

    /* insert this into the dictionary. */
    retval = rbtree_insert(root->dict, &entry->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_entry;
    }

    /* success. */
    goto done;

cleanup_entry:
    release_retval = resource_release(&entry->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_key_value:
    if (NULL != key)
    {
        free(key);
        key = NULL;
    }

    if (NULL != value)
    {
        free(value);
        value = NULL;
    }

done:
    return retval;
}

/**
 * \brief Split a key-value pair entry into a key string and a value string.
 *
 * \param key       Pointer to receive the key string on success.
 * \param value     Pointer to receive the value string on success.
 * \param kvp       The kvp string to split.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status root_dict_split(char** key, char** value, const char* kvp)
{
    status retval;
    char* kvp_equals;

    /* the key, value, and kvp pointers must all be valid. */
    if (NULL == key || NULL == value || NULL == kvp)
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_PARAMETER;
        goto done;
    }

    /* duplicate the key-value pair. */
    char* dup_kvp = strdup(kvp);
    if (NULL == dup_kvp)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* attempt to find the equals sign. */
    for (
        kvp_equals = dup_kvp;
        *kvp_equals != 0 && *kvp_equals != '=';
        ++kvp_equals);

    /* if we did not find the equals sign, this is an invalid kvp. */
    if (*kvp_equals != '=')
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_KVP;
        goto cleanup_dup_kvp;
    }

    /* if there is no key value, this is an invalid kvp. */
    if (dup_kvp == kvp_equals)
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_KVP;
        goto cleanup_dup_kvp;
    }

    /* if there is no value, this is an invalid kvp. */
    const char* pvalue = kvp_equals + 1;
    if (0 == *pvalue)
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_KVP;
        goto cleanup_dup_kvp;
    }

    /* change the equals sign to a NULL. */
    *kvp_equals = 0;

    /* duplicate the key. */
    *key = strdup(dup_kvp);
    if (NULL == *key)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_dup_kvp;
    }

    /* duplicate the value. */
    *value = strdup(pvalue);
    if (NULL == *value)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_key;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_key:
    free(*key);
    *key = NULL;

cleanup_dup_kvp:
    free(dup_kvp);

done:
    return retval;
}

/**
 * \brief Release a kvp entry resource.
 *
 * \param r         The resource to release.
 */
static status root_dict_kvp_release(resource* r)
{
    root_dict_kvp* entry = (root_dict_kvp*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* free the key. */
    if (NULL != entry->key)
    {
        free(entry->key);
    }

    /* free the value. */
    if (NULL != entry->value)
    {
        free(entry->value);
    }

    /* reclaim the memory. */
    return rcpr_allocator_reclaim(alloc, entry);
}

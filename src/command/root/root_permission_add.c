/**
 * \file command/root/root_permission_add.c
 *
 * \brief Add a permission to the root permissions list.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <rcpr/allocator.h>
#include <string.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_slist;
RCPR_IMPORT_resource;

/* forward decls. */
static status root_permission_split(
    char** entity, char** moiety, const char* perm);
static status root_permission_release(resource* r);

/**
 * \brief Add a permission in the form of "entity:moiety" to the permission
 * list.
 *
 * \param root          The command-line root command.
 * \param perm          The permission to add.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int root_permission_add(root_command* root, const char* perm)
{
    status retval, release_retval;
    char* entity = NULL;
    char* moiety = NULL;
    root_permission* entry = NULL;

    /* attempt to split the perm into an entity and moiety. */
    retval = root_permission_split(&entity, &moiety, perm);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* allocate memory for the permission entry. */
    retval =
        rcpr_allocator_allocate(root->alloc, (void**)&entry, sizeof(*entry));
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_moiety;
    }

    /* clear this structure. */
    memset(entry, 0, sizeof(*entry));

    /* initialize this as a resource. */
    resource_init(&entry->hdr, &root_permission_release);

    /* set values. */
    entry->alloc = root->alloc;
    entry->entity = entity;
    entry->moiety = moiety;

    /* entity and moiety are now owned by the entry instance. */
    entity = moiety = NULL;

    /* append this to the permission list. */
    retval = slist_append_tail(root->permissions, &entry->hdr);
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

cleanup_entity_moiety:
    if (NULL != entity)
    {
        free(entity);
        entity = NULL;
    }

    if (NULL != moiety)
    {
        free(moiety);
        moiety = NULL;
    }

done:
    return retval;
}

/**
 * \brief Split a permission entry into an entity string and a moiety string.
 *
 * \param entity        Pointer to receive the entity string on success.
 * \param moiety        Pointer to receive the moiety string on success.
 * \param perm          The permission entry to split.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status root_permission_split(
    char** entity, char** moiety, const char* perm)
{
    status retval;
    char* perm_colon;

    /* the entity, moiety, and perm pointers must all be valid. */
    if (NULL == entity || NULL == moiety || NULL == perm)
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_PARAMETER;
        goto done;
    }

    /* duplicate the perm entry. */
    char* dup_perm = strdup(perm);
    if (NULL == dup_perm)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* attempt to find the colon. */
    for (
        perm_colon = dup_perm;
        *perm_colon != 0 && *perm_colon != ':';
        ++perm_colon);

    /* if we did not find the colon, this is an invalid perm. */
    if (*perm_colon != ':')
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_PERMISSION;
        goto cleanup_dup_perm;
    }

    /* if there is no entity value, then this is an invalid perm. */
    if (dup_perm == perm_colon)
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_PERMISSION;
        goto cleanup_dup_perm;
    }

    /* if there is no moiety, then this is an invalid perm. */
    const char* pmoiety = perm_colon + 1;
    if (0 == *pmoiety)
    {
        retval = VCTOOL_ERROR_COMMANDLINE_BAD_PERMISSION;
        goto cleanup_dup_perm;
    }

    /* change the colon to a NULL. */
    *perm_colon = 0;

    /* duplicate the entity. */
    *entity = strdup(dup_perm);
    if (NULL == *entity)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_dup_perm;
    }

    /* duplicate the moiety. */
    *moiety = strdup(pmoiety);
    if (NULL == *moiety)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_entity;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_entity:
    free(*entity);
    *entity = NULL;

cleanup_dup_perm:
    free(dup_perm);

done:
    return retval;
}

/**
 * \brief Release a root permission entry resource.
 *
 * \param r             The resource to release.
 */
static status root_permission_release(resource* r)
{
    root_permission* entry = (root_permission*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* free the entity if set. */
    if (NULL != entry->entity)
    {
        free(entry->entity);
    }

    /* free the moiety if set. */
    if (NULL != entry->moiety)
    {
        free(entry->moiety);
    }

    /* reclaim the memory. */
    return rcpr_allocator_reclaim(alloc, entry);
}

/**
 * \file command/endorse/endorse_working_set_add_capabilities.c
 *
 * \brief Decode and add the capabilities represented by the given moiety.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Decode and add the capabilities represented by the given moiety.
 *
 * \param set               The current working set.
 * \param alloc             The allocator to use for this operation.
 * \param entity            The entity to use for this operation.
 * \param entity_id         The ID of this entity.
 * \param moiety            The moiety to decode.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_working_set_add_capabilities(
    RCPR_SYM(rbtree)* set, RCPR_SYM(allocator)* alloc, endorse_entity* entity,
    const RCPR_SYM(rcpr_uuid)* entity_id, const char* moiety)
{
    status retval;
    endorse_role* role;
    endorse_verb* verb;

    /* check the roles tree for the moiety. */
    retval = rbtree_find((resource**)&role, entity->roles, moiety);
    if (STATUS_SUCCESS == retval)
    {
        /* add all roles associated with this verb to the working set. */
        retval =
            endorse_working_set_add_role_capabilities(
                set, alloc, entity_id, role);
        goto done;
    }

    /* check the verbs tree for the moiety. */
    retval = rbtree_find((resource**)&verb, entity->verbs, moiety);
    if (STATUS_SUCCESS == retval)
    {
        /* add this verb to the working set. */
        retval =
            endorse_working_set_add_verb_capability(
                set, alloc, entity_id, verb);
        goto done;
    }

    /* if we've made it here, then the moiety is unknown. */
    fprintf(stderr, "Unknown role or verb %s:%s.\n", entity->id, moiety);
    retval = VCTOOL_ERROR_ENDORSE_UNKNOWN_ROLE_OR_VERB;
    goto done;

done:
    return retval;
}

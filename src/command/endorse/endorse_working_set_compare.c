/**
 * \file command/endorse/endorse_working_set_compare.c
 *
 * \brief Compare two working set values.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#include "endorse_internal.h"

/**
 * \brief Compare two opaque working set values.
 *
 * \param context       Unused.
 * \param lhs           The left-hand side of the comparison.
 * \param rhs           The right-hand side of the comparison.
 *
 * \returns an integer value representing the comparison result.
 *      - RCPR_COMPARE_LT if \p lhs &lt; \p rhs.
 *      - RCPR_COMPARE_EQ if \p lhs == \p rhs.
 *      - RCPR_COMPARE_GT if \p lhs &gt; \p rhs.
 */
RCPR_SYM(rcpr_comparison_result) endorse_working_set_compare(
    void* /*context*/, const void* lhs, const void* rhs)
{
    endorse_working_set_key* l = (endorse_working_set_key*)lhs;
    endorse_working_set_key* r = (endorse_working_set_key*)rhs;

    /* compare the two keys. */
    int retval = memcmp(l, r, sizeof(endorse_working_set_key));

    /* decode the result. */
    if (retval < 0)
    {
        return RCPR_COMPARE_LT;
    }
    else if (retval > 0)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}

/**
 * \file include/vctool/control.h
 *
 * \brief Control-flow helpers.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#pragma once

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Try to perform an operation, or jump to the given label on failure.
 */
#define TRY_OR_FAIL(op, label) \
    do { \
        retval = (op); \
        if (STATUS_SUCCESS != retval) \
        { \
            goto label; \
        } \
    } while (0)

/**
 * \brief Clean up the given resource, or merge cleanup failure code with return
 * value.
 */
#define CLEANUP_OR_CASCADE(resource) \
    do { \
        release_retval = resource_release(resource); \
        if (STATUS_SUCCESS != release_retval) \
        { \
            retval = release_retval; \
        } \
    } while (0)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

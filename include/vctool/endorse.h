/**
 * \file include/vctool/endorse.h
 *
 * \brief Endorse config related header.
 *
 * \copyright 2022 Velo Payments.  See License.txt for license terms.
 */

#pragma once

#include <rcpr/allocator.h>
#include <rcpr/rbtree.h>
#include <rcpr/resource/protected.h>
#include <stdint.h>
#include <vpr/uuid.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief An endorse config entity.
 */
typedef struct endorse_entity endorse_entity;

struct endorse_entity
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    const char* id;
};

/**
 * \brief Root of the endorse configuration AST.
 */
typedef struct endorse_config endorse_config;

struct endorse_config
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    int reference_count;
    RCPR_SYM(rbtree)* entities;
};

/**
 * \brief Union for the endorse config parser.
 */
typedef union endorse_config_val endorse_config_val;

union endorse_config_val
{
    int64_t number;
    const char* string;
    vpr_uuid id;
    endorse_config* config;
    RCPR_SYM(rbtree)* entities;
};

/**
 * \brief The endorse config context structure is used to provide user overrides
 * for methods and a user context pointer to the parser.
 */
typedef struct endorse_config_context endorse_config_context;

/**
 * \brief This callback provides a means for an error to be set by the caller.
 */
typedef
void (*endorse_config_set_error_fn)(
    endorse_config_context*, const char*);

/** 
 * \brief Configuration value callback.
 */
typedef
void (*endorse_config_val_callback_fn)(
    endorse_config_context*, endorse_config*);

struct endorse_config_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    endorse_config_set_error_fn set_error;
    endorse_config_val_callback_fn val_callback;
    void* user_context;
};

/* helper to link our value to Bison. */
#define YYSTYPE endorse_config_val
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif /*YY_TYPEDEF_YY_SCANNER_T*/

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif
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
#include <vccrypt/buffer.h>
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
    int reference_count;
    const char* id;
    bool id_declared;
    RCPR_SYM(rbtree)* verbs;
    RCPR_SYM(rbtree)* roles;
};

/**
 * \brief An endorse config verb.
 */
typedef struct endorse_verb endorse_verb;

struct endorse_verb
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    int reference_count;
    const char* verb;
    vpr_uuid verb_id;
};

/**
 * \brief An endorse role verb.
 */
typedef struct endorse_role_verb endorse_role_verb;

struct endorse_role_verb
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    int reference_count;
    const char* verb_name;
    endorse_verb* verb;
};

/**
 * \brief An endorse role.
 */
typedef struct endorse_role endorse_role;

struct endorse_role
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    bool type_complete;
    int reference_count;
    const char* name;
    const char* extends_role_name;
    endorse_role* extends_role;
    RCPR_SYM(rbtree)* verbs;
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
    char* string;
    vpr_uuid* id;
    endorse_config* config;
    RCPR_SYM(rbtree)* entities;
    RCPR_SYM(rbtree)* roles;
    endorse_entity* entity;
    endorse_verb* verb;
    RCPR_SYM(rbtree)* verbs;
    RCPR_SYM(rbtree)* role_verbs;
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

/**
 * \brief Create an endorse context using the given set_error, value callback,
 * and user context values.
 *
 * \note This function creates a resource, but does not add resource management
 * for the user context. This is up to the caller.
 *
 * \param context       Pointer to receive the new context.
 * \param alloc         The allocator to use for this operation.
 * \param set_error     The set_error function callback.
 * \param val_callback  The value callback.
 * \param user_context  The user context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_create_ex(
    endorse_config_context** context, RCPR_SYM(allocator)* alloc,
    endorse_config_set_error_fn set_error,
    endorse_config_val_callback_fn val_callback, void* user_context);

/**
 * \brief Create a default endorse config context that saves a list of errors
 * and saves the endorse config root.
 *
 * \param context       Pointer to receive the default context.
 * \param alloc         The allocator to use to create this context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_create_default(
    endorse_config_context** context, RCPR_SYM(allocator)* alloc);

/**
 * \brief Get the number of error messages from the default endorse config.
 *
 * \param context       Pointer to the config context.
 *
 * \returns The number of error messages.
 */
size_t endorse_config_default_context_get_error_message_count(
    const endorse_config_context* context);

/**
 * \brief Get the endorse config root.
 *
 * \param context       Pointer to the config context.
 *
 * \returns The endorse config root.
 */
const endorse_config* endorse_config_default_context_get_endorse_config_root(
    const endorse_config_context* context);

/**
 * \brief Get the Nth error message from the default endorse config.
 *
 * \param msg           Pointer to receive the error message pointer. This error
 *                      message is owned by the config context and must not be
 *                      freed.
 * \param context       Pointer to receive the default context.
 * \param index         The error message index.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_config_default_context_get_error_message(
    const char** msg, const endorse_config_context* context, int index);

/**
 * \brief Parse a config file read into memory as a buffer.
 *
 * \param root          Pointer to receive the root of the AST.
 * \param context       The endorse config context for this parse.
 * \param input         The input buffer to parse. Must be ASCIIZ.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_parse(
    endorse_config** root, endorse_config_context* context,
    const vccrypt_buffer_t* input);

/**
 * \brief Analyze the AST produced by the endorse file parser and finish
 * populating the AST with relevant data.
 *
 * \param context       The endorse config context for this parse.
 * \param root          The AST root to analyze.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status endorse_analyze(endorse_config_context* context, endorse_config* root);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

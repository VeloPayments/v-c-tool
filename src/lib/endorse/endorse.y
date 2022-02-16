/**
 * \file lib/endorse/endorse.y
 *
 * \brief Parser for vctool endorse config files.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

%{
#include <string.h>
#include <vctool/endorse.h>
#include <vpr/parameters.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Helper macro for passing an error condition to the caller and breaking
 * out of the parse.
 */
#define CONFIG_ERROR(s) \
    context->set_error(context, (s)); \
    return NULL

/**
 * \brief Helper macro for breaking out of the parse if a NULL pointer is
 * returned by the helper method.
 */
#define MAYBE_ASSIGN(lhs, rhs) \
    do { \
        typeof (lhs) x = (rhs); \
        if (NULL == x) YYACCEPT; \
        (lhs) = x; \
    } while (false)

/* forward decls */
int yylex();
int yyerror(
    yyscan_t scanner, endorse_config_context* context, const char*);
static endorse_config* new_endorse_config(endorse_config_context* context);
static status endorse_config_resource_release(resource* r);
%}

/* use the full pure API for Bison. */
%define api.pure full
/* We pass the scanner to the lexer. */
%lex-param {yyscan_t scanner}
/* We pass the scanner to Bison. */
%parse-param {yyscan_t scanner}
/* We pass our config context structure to Bison. */
%parse-param {endorse_config_context* context}

/* Tokens. */
%token <string> COMMA
%token <string> LBRACE
%token <string> ENTITIES
%token <string> EXTENDS
%token <string> FOR
%token <string> IDENTIFIER
%token <string> INVALID
%token <string> RBRACE
%token <string> ROLES
%token <id> UUID
%token <id> UUID_INVALID
%token <string> VERBS

%type <config> endorse

%%

/* Base endorse config rule. */
endorse : {
            /* create a new config. */
            MAYBE_ASSIGN($$, new_endorse_config(context));
            context->val_callback(context, $$); }
    ;

%%

/**
 * \brief Create a new configuration structure.
 */
static endorse_config* new_endorse_config(endorse_config_context* context)
{
    status retval;
    endorse_config* cfg = NULL;

    retval =
        rcpr_allocator_allocate(context->alloc, (void**)&cfg, sizeof(*cfg));
    if (STATUS_SUCCESS != retval)
    {
        CONFIG_ERROR("Out of memory in new_endorse_config().");
    }

    /* set up config structure. */
    memset(cfg, 0, sizeof(*cfg));
    resource_init(&cfg->hdr, &endorse_config_resource_release);
    cfg->alloc = context->alloc;

    return cfg;
}

/**
 * \brief Release an endorse config resource.
 */
static status endorse_config_resource_release(resource* r)
{
    endorse_config* cfg = (endorse_config*)r;

    rcpr_allocator* alloc = cfg->alloc;

    /* clear memory. */
    memset(cfg, 0, sizeof(*cfg));

    return
        rcpr_allocator_reclaim(alloc, cfg);
}

/**
 * \brief Set the error for the config structure.
 */
int yyerror(
    yyscan_t /*scanner*/, endorse_config_context* context, const char* msg)
{
    context->set_error(context, msg);

    return 1;
}

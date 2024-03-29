/**
 * \file lib/endorse/endorse.l
 *
 * \brief Lexical scanner for vctool endorse config files.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

%{
#include <vctool/endorse.h>
#include "endorse.tab.h"

FILE* vctool_endorse_set_input_filedescriptor(
    yyscan_t scanner, int fd, void* state);
%}

%option reentrant
%option bison-bridge
%option noyywrap nounput noinput

%%

[{] {
    /* lbrace token */
    return LBRACE;
}

[}] {
    /* rbrace token */
    return RBRACE;
}

[,] {
    /* comma token */
    return COMMA;
}

entities {
    /* entities keyword */
    return ENTITIES;
}

extends {
    /* extends keyword */
    return EXTENDS;
}

for {
    /* for keyword */
    return FOR;
}

roles {
    /* roles keyword */
    return ROLES;
}

verbs {
    /* verbs keyword */
    return VERBS;
}

[A-Za-z_][A-Za-z0-9_]* {
    /* identifier token */
    yylval->string = strdup(yytext);
    return IDENTIFIER;
}

([0-9a-fA-F]){8}[-]([0-9a-fA-F]){4}[-]([0-9a-fA-F]){4}[-]([0-9a-fA-F]){4}[-]([0-9a-fA-F]){12} {
    yylval->id = malloc(sizeof(*yylval->id));
    memset(yylval->id, 0, sizeof(*yylval->id));
    if (STATUS_SUCCESS != vpr_uuid_from_string(yylval->id, yytext))
    {
        return UUID_INVALID;
    }

    return UUID;
}

[[:space:]] /* ignore whitespace */

. {
    /* invalid token */
    yylval->string = strdup(yytext);
    return INVALID;
}

%%

/**
 * \brief Set the scanner to read from a Unix file descriptor.
 *
 * \param scanner       The scanner context structure.
 * \param fd            The file descriptor to use.
 * \param state         The buffer state structure used by the scanner.
 *
 * \returns the minted file handle, to be closed by the caller when scanning
 * is complete.
 */
FILE* vctool_endorse_set_input_filedescriptor(
    yyscan_t scanner, int fd, void* st)
{
    YY_BUFFER_STATE* state = (YY_BUFFER_STATE*)st;

    FILE* f = fdopen(fd, "r");

    if (NULL != f)
    {
        *state = yy_create_buffer(f, YY_BUF_SIZE, scanner);
        yy_switch_to_buffer(*state, scanner);
    }

    return f;
}

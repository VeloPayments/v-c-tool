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
    yylval->string = "{";
    return LBRACE;
}

[}] {
    /* rbrace token */
    yylval->string = "}";
    return RBRACE;
}

[,] {
    /* comma token */
    yylval->string = ",";
    return COMMA;
}

entities {
    /* entities keyword */
    yylval->string = "entities";
    return ENTITIES;
}

extends {
    /* extends keyword */
    yylval->string = "extends";
    return EXTENDS;
}

for {
    /* for keyword */
    yylval->string = "for";
    return FOR;
}

roles {
    /* roles keyword */
    yylval->string = "roles";
    return ROLES;
}

verbs {
    /* verbs keyword */
    yylval->string = "verbs";
    return VERBS;
}

[A-Za-z_][A-Za-z0-9_]* {
    /* identifier token */
    yylval->string = strdup(yytext);
    return IDENTIFIER;
}

([0-9a-fA-F]){8}[-]([0-9a-fA-F]){4}[-]([0-9a-fA-F]){4}[-]([0-9a-fA-F]){4}[-]([0-9a-fA-F]){12} {
    memset(&yylval->id, 0, sizeof(yylval->id));
    if (STATUS_SUCCESS != vpr_uuid_from_string(&yylval->id, yytext))
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

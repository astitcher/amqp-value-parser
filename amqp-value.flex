/* Flex scanner for AMQP 1.0 value parser */

%option bison-bridge reentrant 8bit
%option prefix="pn_parser_"
%option noyywrap

%option noinput nounput
%option noyyget_lval noyyset_lval

%{
    #include <stdint.h>
    #include <stdlib.h>
    #include <proton/codec.h>
    #include "amqp-value.tab.h"
%}

WS      [ \f\r\t\v]
DIGIT   [0-9]
ALNUM   [a-zA-Z0-9]
TOK     [{}\[\]:,=@]
SIGN    ("+"|"-")

%%
        /* Ignore whitespace */
{WS}
        /* single character tokens */
{TOK}       { return yytext[0]; }
"true"      { return PN_TOK_TRUE; }
"false"     { return PN_TOK_FALSE; }
"null"      { return PN_TOK_NULL; }

b\"([^\"]|\\\")*\" { yylval->t_str.bytes = yytext+2; yylval->t_str.size = yyleng-3; return PN_TOK_BINARY; }
\"([^\"]|\\\")*\"  { yylval->t_str.bytes = yytext+1; yylval->t_str.size = yyleng-2; return PN_TOK_STRING; }

{SIGN}?{DIGIT}+    { yylval->t_int = atoll(yytext); return PN_TOK_INT; }
{SIGN}?({DIGIT}+"."|{DIGIT}*("."{DIGIT}+)?)([eE]{DIGIT}+)? {
    yylval->t_float = atof(yytext); return PN_TOK_FLOAT;
}

{ALNUM}+    { yylval->t_str.bytes = yytext; yylval->t_str.size = yyleng; return PN_TOK_ID; }

.           { return PN_TOK_ERROR; }
%%


/* Flex scanner for AMQP 1.0 value parser */

%option bison-bridge reentrant 8bit
%option prefix="pn_parser_"
%option noyywrap

%option noinput nounput
%option noyyget_lval noyyset_lval

%{
    #include <stdint.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <proton/codec.h>
    #include "amqp-value.tab.h"

    int pni_process_string_escapes(size_t size, char* s);
%}

WS      [ \f\r\t\v\n]
DIGIT   [0-9]
ALNUM   [-a-zA-Z0-9]
TOK     [{}()\[\]:,=@]
SIGN    [-+]
STRING  \"([^\"]|\\\")*\"
%%
        /* Ignore whitespace */
{WS}
        /* single character tokens */
{TOK}       { return yytext[0]; }
"=>"        { return PN_TOK_DARROW; }
"true"      { return PN_TOK_TRUE; }
"false"     { return PN_TOK_FALSE; }
"null"      { return PN_TOK_NULL; }

b{STRING}   { yylval->t_str.bytes = yytext+2;
              yylval->t_str.size = pni_process_string_escapes(yyleng-3, yytext+2);
              return PN_TOK_BINARY; }
{STRING}    { yylval->t_str.bytes = yytext+1;
              yylval->t_str.size = pni_process_string_escapes(yyleng-2, yytext+1);
              return PN_TOK_STRING; }

{SIGN}?{DIGIT}+    { yylval->t_int = atoll(yytext); return PN_TOK_INT; }
{SIGN}?({DIGIT}+"."|{DIGIT}*("."{DIGIT}+)?)([eE]{DIGIT}+)? {
    yylval->t_float = atof(yytext); return PN_TOK_FLOAT;
}

{ALNUM}+    { yylval->t_str.bytes = yytext; yylval->t_str.size = yyleng; return PN_TOK_ID; }

.           { yylval->t_err = *yytext; return PN_TOK_ERROR; }
%%

/* Process escape characters in string
 *
 * We modiy the input string inline as we know that escape characters
 * can only make the string shorter and in the context of flex
 * yytext can be safely modified.
 *
 * Escapes that aren't understood are left as-is
 *
 * @return number of processed characters
 */
int pni_process_string_escapes(size_t size, char* s)
{
    int count = 0;
    int value = 0;
    enum { REG, OCT, HEX } state = REG;

    char* src = s;
    char* dst = s;
    char* end = s+size;

    for ( ; src<end; ++src) {
        switch (state) {
        case REG:
            if ( *src=='\\' && end-src>=2 ) {
                ++src;
                switch (*src) {
                default:
                    --src;
                case '\\': case '\"': case '\'':
                    break;
                case 'a': *dst++ = '\a'; continue;
                case 'b': *dst++ = '\b'; continue;
                case 'f': *dst++ = '\f'; continue;
                case 'n': *dst++ = '\n'; continue;
                case 'r': *dst++ = '\r'; continue;
                case 't': *dst++ = '\t'; continue;
                case 'v': *dst++ = '\v'; continue;
                case 'x':
                    value = 0;
                    count = 0;
                    state = HEX;
                    continue;
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    value = *src - '0';
                    count = 1;
                    state = OCT;
                    continue;
                }
            }
            *dst++ = *src;
            break;
        case OCT:
            if ( *src>='0' && *src<='7' && count<3 ) {
                value *= 8;
                value += *src-'0';
                ++count;
            } else {
                *dst++ = value;
                state = REG;
                --src;
            }
            break;
        case HEX:
            if ( isxdigit(*src) && count<2 ) {
                value *= 16;
                value += isdigit(*src) ? *src-'0' : toupper(*src)-'A'+10;
                ++count;
            } else if ( count>0 ) {
                *dst++ = value;
                state = REG;
                --src;
            } else {
                *dst++ = '\\';
                *dst++ = 'x';
                state = REG;
                --src;
            }
            break;
         }
    }
    switch (state) {
    case HEX:
        if ( count==0 ) {
            *dst++ = '\\';
            *dst++ = 'x';
            break;
        }
    case OCT:
        *dst++ = value;
    case REG:
        break;
    }
    return dst-s;
}

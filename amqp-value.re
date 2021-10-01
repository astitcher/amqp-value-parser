#include <proton/codec.h>

#include "amqp-value.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

static int pni_process_string_escapes(size_t size, char* s);

typedef struct {
    size_t size;
    char*  bytes;
} ByteRange;

#define RETURN_UPDATE_NOTOK(x) do { input->bytes = p; input->size = e-p; return (x); } while (false)
#define RETURN_UPDATE(x) do { tok->start = t; tok->size = p-t; input->bytes = p; input->size = e-p; return (x); } while (false)

static int pni_parser_scan(ByteRange* input, pn_bytes_t* tok)
{
    char* p = input->bytes;        /* cursor */
    char* e = p+input->size;       /* limit */
    char* t = p;        /* record real token start ignoring ws */
    char* m;

    while (*p) {
        t = p;
    /*!re2c
    re2c:define:YYCTYPE  = "char";
    re2c:define:YYCURSOR = p;
    re2c:define:YYMARKER = m;
    re2c:yyfill:enable = 0;

    WS     = [ \f\r\t\v\n];
    DIGIT  = [0-9];
    NZDIGIT= [1-9];
    ODIGIT = [0-7];
    BDIGIT = [0-1];
    HDIGIT = [0-9a-fA-F];
    ALNUM  = [-a-zA-Z0-9];
    SIGN   = [-+];
    STRING = ["]([^"]|"\\\"")*["];

    // Ignore whitespace
    WS          { continue; }

    "("         { RETURN_UPDATE(PN_TOK_LPAREN); }
    ")"         { RETURN_UPDATE(PN_TOK_RPAREN); }
    "{"         { RETURN_UPDATE(PN_TOK_LBRACE); }
    "}"         { RETURN_UPDATE(PN_TOK_RBRACE); }
    "["         { RETURN_UPDATE(PN_TOK_LBRACKET); }
    "]"         { RETURN_UPDATE(PN_TOK_RBRACKET); }
    ":"         { RETURN_UPDATE(PN_TOK_COLON); }
    ","         { RETURN_UPDATE(PN_TOK_COMMA); }
    "="         { RETURN_UPDATE(PN_TOK_EQUAL); }
    "@"         { RETURN_UPDATE(PN_TOK_AT); }
    "=>"        { RETURN_UPDATE(PN_TOK_DARROW); }

    "b" STRING  { tok->start = t+2;
                  tok->size = pni_process_string_escapes(p-t-3, t+2);
                  RETURN_UPDATE_NOTOK(PN_TOK_BINARY); }
    STRING      { tok->start = t+1;
                  tok->size = pni_process_string_escapes(p-t-2, t+1);
                  RETURN_UPDATE_NOTOK(PN_TOK_STRING); }

    ("0b" | "0B") BDIGIT+ { t+=2; RETURN_UPDATE(PN_TOK_BINT); }
    "0" ODIGIT*           { t+=1; RETURN_UPDATE(PN_TOK_OINT); }
    ("0x" | "0X") HDIGIT+ { t+=2; RETURN_UPDATE(PN_TOK_HINT); }
    SIGN? NZDIGIT DIGIT*  { RETURN_UPDATE(PN_TOK_INT); }

    SIGN? DIGIT+ "." ([eE] SIGN? DIGIT+)?        { RETURN_UPDATE(PN_TOK_FLOAT); }
    SIGN? DIGIT* "." DIGIT+ ([eE] SIGN? DIGIT+)? { RETURN_UPDATE(PN_TOK_FLOAT); }
    SIGN? DIGIT+ [eE] SIGN? DIGIT+               { RETURN_UPDATE(PN_TOK_FLOAT); }

    ALNUM+      { RETURN_UPDATE(PN_TOK_ID); }

    [^]         { RETURN_UPDATE(-1); }
    */
    }
    RETURN_UPDATE(0);
}

/* Process escape characters in string
 *
 * We modify the input string inline as we know that escape characters
 * can only make the string shorter
 *
 * Escapes that aren't understood are left as-is
 *
 * @return number of processed characters
 */
static int pni_process_string_escapes(size_t size, char* s)
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

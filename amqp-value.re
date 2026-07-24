// re2c --lang c
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
    re2c:define:YYLIMIT = e;
    re2c:yyfill:enable = 0;
    re2c:eof = 0;

    WS     = [ \f\r\t\v\n];
    DIGIT  = [0-9];
    NZDIGIT= [1-9];
    ODIGIT = [0-7];
    BDIGIT = [0-1];
    HDIGIT = [0-9a-fA-F];
    ALNUM  = [-a-zA-Z0-9];
    SIGN   = [-+];
    STRING = ["] ([^"] | [\\]["])* ["];

    // Ignore whitespace
    WS          { continue; }

    "("         { RETURN_UPDATE(PN_TOK_LPAREN); }
    ")"         { RETURN_UPDATE(PN_TOK_RPAREN); }
    "{"         { RETURN_UPDATE(PN_TOK_LBRACE); }
    "}"         { RETURN_UPDATE(PN_TOK_RBRACE); }
    "["         { RETURN_UPDATE(PN_TOK_LBRACKET); }
    "]"         { RETURN_UPDATE(PN_TOK_RBRACKET); }
    "<"         { RETURN_UPDATE(PN_TOK_LESS); }
    ">"         { RETURN_UPDATE(PN_TOK_GREATER); }
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

    [^]         { return(-1); }
    $           { return(-1); }
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
    char* p = s;
    char* e = s+size;
    char* d = s;
    char* m;
    char* start;

    while (p < e) {
        start = p;
    /*!re2c
    re2c:define:YYCTYPE  = "char";
    re2c:define:YYCURSOR = p;
    re2c:define:YYMARKER = m;
    re2c:define:YYLIMIT = e;
    re2c:yyfill:enable = 0;
    re2c:eof = 0;

    OCT = [0-7];
    HEX = [0-9a-fA-F];

    "\\\\"          { *d++ = '\\'; continue; }
    "\\\""          { *d++ = '"';  continue; }
    "\\'"           { *d++ = '\''; continue; }
    "\\a"           { *d++ = '\a'; continue; }
    "\\b"           { *d++ = '\b'; continue; }
    "\\f"           { *d++ = '\f'; continue; }
    "\\n"           { *d++ = '\n'; continue; }
    "\\r"           { *d++ = '\r'; continue; }
    "\\t"           { *d++ = '\t'; continue; }
    "\\v"           { *d++ = '\v'; continue; }
    "\\x" HEX HEX?  { unsigned int value = 0; char* q; for (q = start+2; q < p; ++q) value = value*16 + (isdigit(*q) ? *q-'0' : toupper(*q)-'A'+10); *d++ = (char)value; continue; }
    "\\" OCT {1,3}  { unsigned int value = 0; char* q; for (q = start+1; q < p; ++q) value = value*8 + (*q - '0'); *d++ = (char)value; continue; }
    [^]             { *d++ = *start; continue; }
    $               { break; }
    */
    }
    return d-s;
}

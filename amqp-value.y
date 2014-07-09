/*%define api.pure*/
/*%define api.prefix pn_parser_*/
%pure_parser
%name-prefix="pn_parser_"

%{
    #include <stdint.h>
   
%}

%token <t_str>   PN_TOK_BINARY
%token <t_str>   PN_TOK_STRING
%token <t_str>   PN_TOK_ID
%token <t_float> PN_TOK_FLOAT
%token <t_int>   PN_TOK_INT
%token PN_TOK_TRUE  "true"
%token PN_TOK_FALSE "false"
%token PN_TOK_NULL  "null"
%token PN_TOK_ERROR "<illegal character>"

%union {
    const char* t_str;
    uint64_t    t_int;
    double      t_float;
}

/* void* is really yyscan_t, but it can't be defined here */
%parse-param {void* scanner}
%lex-param {yyscan_t scanner}

%{
    #include "amqp-value.lex.h"
    void pn_parser_error(yyscan_t, const char*);
%}
%%

value
: described_value       { printf("described_value\n"); }
| map                   { printf("map\n"); } 
| list                  { printf("list\n"); }
| symbol                { printf("symbol\n"); }
| PN_TOK_BINARY         { printf("binary\n"); }
| PN_TOK_STRING         { printf("string\n"); }
| PN_TOK_INT            { printf("int\n"); }
| PN_TOK_FLOAT          { printf("float\n"); }
| "true"                { printf("true\n"); }
| "false"               { printf("false\n"); }
| "null"                { printf("null\n"); }
;

descriptor
: value
;

described_value
: '@' descriptor value
;

map_key
: value
;

map_entry
: map_key '=' value
| map_key ':' value
;

map_list
: map_entry
| map_list ',' map_entry
;

map
: '{' '}'
| '{' map_list '}'
;


list_list
: value
| list_list ',' value
;

list
: '[' ']'
| '[' list_list ']'
;

symbol
: ':' PN_TOK_STRING
| ':' PN_TOK_ID
;

%%

#include <stdio.h>

int main()
{
    yyscan_t scanner;
    pn_parser_lex_init(&scanner);
    int r = pn_parser_parse(scanner);
    printf(r==0 ? "succeeded\n" : "failed\n");
    return 0;
}

void pn_parser_error(yyscan_t scanner, const char* error)
{
    printf("Error: %s\n", error);
}


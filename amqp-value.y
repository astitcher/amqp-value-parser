%define api.pure
/*%define api.prefix "pn_parser_"*/
/*%pure_parser*/
%name-prefix="pn_parser_"

%{
    #include <stdint.h>

    #include <proton/codec.h>
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
    struct {
        size_t size;
        char*  bytes;
    }           t_str;
    uint64_t    t_int;
    double      t_float;
}

/* void* is really yyscan_t, but it can't be defined here */
%parse-param {void* scanner}
%parse-param {pn_data_t* data}
%lex-param {yyscan_t scanner}

%{
    #include "amqp-value.lex.h"
    void pn_parser_error(yyscan_t, pn_data_t*, const char*);
%}
%%

value
: described_value
| map
| list
| symbol
| PN_TOK_BINARY         { pn_data_put_binary(data, pn_bytes($1.size, $1.bytes)); }
| PN_TOK_STRING         { pn_data_put_string(data, pn_bytes($1.size, $1.bytes)); }
| PN_TOK_INT            { pn_data_put_long(data, $1); }
| PN_TOK_FLOAT          { pn_data_put_float(data, $1); }
| "true"                { pn_data_put_bool(data, true); }
| "false"               { pn_data_put_bool(data, false); }
| "null"                { pn_data_put_null(data); }
;

descriptor
: value
;

described_value
: '@'                   { pn_data_put_described(data); pn_data_enter(data); }
  descriptor value      { pn_data_exit(data); }
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
: '{' '}'               { pn_data_put_map(data); }
| '{'                   { pn_data_put_map(data); pn_data_enter(data); }
  map_list '}'          { pn_data_exit(data); }
;


list_list
: value
| list_list ',' value
;

list
: '[' ']'               { pn_data_put_list(data); }
| '['                   { pn_data_put_list(data); pn_data_enter(data); } 
   list_list ']'        { pn_data_exit(data); }
;

symbol
: ':' PN_TOK_STRING     { pn_data_put_symbol(data, pn_bytes($2.size, $2.bytes)); }
| ':' PN_TOK_ID         { pn_data_put_symbol(data, pn_bytes($2.size, $2.bytes)); }
;

%%

#include <stdio.h>

int main()
{
    pn_data_t* data = pn_data(16);

    yyscan_t scanner;
    pn_parser_lex_init(&scanner);
    int r = pn_parser_parse(scanner, data);
    pn_parser_lex_destroy(scanner);

    pn_data_rewind(data);
    pn_data_print(data);
    /* pn_data_dump() has bad bug until 0.8 with complex types */
    /*pn_data_rewind(data); */
    /*pn_data_dump(data); */
    pn_data_free(data);    

    printf("\n");
    printf(r==0 ? "succeeded\n" : "failed\n");
    return 0;
}

void pn_parser_error(yyscan_t scanner, pn_data_t* data, const char* error)
{
    pn_data_clear(data);
    printf("Error: %s\n", error);
}


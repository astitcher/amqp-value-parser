%define api.pure
%name-prefix="pn_parser_"
%error-verbose

%code requires {
    #include <stdint.h>

    #include <proton/codec.h>
}

%token <t_str>   PN_TOK_BINARY "Binary"
%token <t_str>   PN_TOK_STRING "String"
%token <t_str>   PN_TOK_ID     "Identifier"
%token <t_float> PN_TOK_FLOAT  "Float"
%token <t_int>   PN_TOK_INT    "Integer"
%token <t_uint>  PN_TOK_UINT   "UInteger"
%token           PN_TOK_DARROW "=>"
%token           PN_TOK_TRUE   "true"
%token           PN_TOK_FALSE  "false"
%token           PN_TOK_NULL   "null"
%token <t_err>   PN_TOK_ERROR  "<illegal character>"

%union {
    struct {
        size_t size;
        char*  bytes;
    }           t_str;
    char        t_err;
    int64_t     t_int;
    uint64_t    t_uint;
    double      t_float;
}

/* void* is really yyscan_t, but it can't be defined here */
%parse-param {void* scanner}
%parse-param {pn_data_t* data}
%lex-param {yyscan_t scanner}

%code provides {
    extern int pn_data_parse(pn_data_t* data, const char* s);
}

%code {
    #include "amqp-value.lex.h"
    static void pn_parser_error(yyscan_t, pn_data_t*, const char*);
}

%start value

%%

value
: described_value
| map
| list
| simple_value
;

simple_value
: symbol
| PN_TOK_INT            { pn_data_put_long(data, $1); }
| PN_TOK_UINT           { pn_data_put_ulong(data, $1); }
| PN_TOK_BINARY         { pn_data_put_binary(data, pn_bytes($1.size, $1.bytes)); }
| PN_TOK_STRING         { pn_data_put_string(data, pn_bytes($1.size, $1.bytes)); }
| PN_TOK_FLOAT          { pn_data_put_double(data, $1); }
| "true"                { pn_data_put_bool(data, true); }
| "false"               { pn_data_put_bool(data, false); }
| "null"                { pn_data_put_null(data); }
;

descriptor_value
: symbol
| PN_TOK_INT            { pn_data_put_ulong(data, $1); }
| PN_TOK_UINT           { pn_data_put_ulong(data, $1); }
;

label
: PN_TOK_ID
;

descriptor
: descriptor_value
| label '(' descriptor_value ')'
;

described_value
: '@'                   { pn_data_put_described(data); pn_data_enter(data); }
  descriptor described_value_value { pn_data_exit(data); }
;

described_value_value
: simple_value
| map
| list
;

map_key
: simple_value
;

map_entry
: map_key '=' value
| map_key "=>" value
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

list_entry
: value
| label '=' value
;

list_list
: list_entry
| list_list ',' list_entry
;

list
: '[' ']'               { pn_data_put_list(data); }
| '['                   { pn_data_put_list(data); pn_data_enter(data); } 
   list_list ']'        { pn_data_exit(data); }
;

symbol
: ':' PN_TOK_STRING     { pn_data_put_symbol(data, pn_bytes($2.size, $2.bytes)); }
| ':' PN_TOK_ID         { pn_data_put_symbol(data, pn_bytes($2.size, $2.bytes)); }
| PN_TOK_ID             { pn_data_put_symbol(data, pn_bytes($1.size, $1.bytes)); }
;

%%

static void pn_parser_error(yyscan_t scanner, pn_data_t* data, const char* error)
{
    pn_data_clear(data);
    pn_error_set(pn_data_error(data), PN_ERR, error);
}

int pn_data_parse_string(pn_data_t* data, const char* s)
{
    yyscan_t scanner;
    pn_parser_lex_init(&scanner);
    YY_BUFFER_STATE buffer = pn_parser__scan_string(s, scanner);
    int r = pn_parser_parse(scanner, data);
    pn_parser__delete_buffer(buffer, scanner);
    pn_parser_lex_destroy(scanner);

    return r;
}

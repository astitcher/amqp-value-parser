#include <proton/codec.h>

#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

extern int pn_data_parse(pn_data_t* data, size_t len, const char* s);
extern int pn_data_parse_string(pn_data_t* data, const char* s);
extern void pn_value_dump(pn_bytes_t frame, pn_string_t *output);

void hexdump(size_t size, const char* buffer)
{
    const char* c;
    for (c=buffer; c<buffer+size; c++) {
        printf("%02hhx", *c);
    }
    printf("\n");
}

void process(pn_data_t* data, const char* str)
{
    pn_data_clear(data);
    pn_error_clear(pn_data_error(data));

    int r = pn_data_parse_string(data, str);

    if (r==0) {
        pn_data_print(data);
        printf("\n");

        char buffer[1024];
        ssize_t s = pn_data_encode(data, buffer, sizeof(buffer));
        printf("Encoded: %zd bytes:\n", s);
        hexdump(s, buffer);

        pn_string_t* out = pn_string("");
        pn_value_dump((pn_bytes_t){.size=s,.start=buffer}, out);
        printf("%s\n", pn_string_get(out));
    } else {
        printf("Failed: %s\n", pn_error_text(pn_data_error(data)));
    }
}

int main(int argc, const char* argv[])
{

    pn_data_t* data = pn_data(16);

    int i;
    for (i=1; i<argc; ++i) {
        process(data, argv[i]);
    }

    /* If we had no arguments then read from stdin */
    if (argc==1) {
      char* buffer = readline(">> ");
      while (buffer) {
        add_history(buffer);
        process(data, buffer);
        free(buffer);
        buffer = readline(">> ");
      }
      free(buffer);
    }

    pn_data_free(data);

    return 0;
}

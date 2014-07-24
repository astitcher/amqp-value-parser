#include <proton/codec.h>
#include <proton/engine.h>

#include <stdio.h>
#include <stdlib.h>

extern int pn_data_parse(pn_data_t* data, size_t len, const char* s);
extern int pn_data_parse_string(pn_data_t* data, const char* s);

void hexdump(size_t size, char* buffer)
{
    char* c;
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
        int s = pn_data_encode(data, buffer, 1024);

        printf("Encoded: %d bytes:\n", s);
        hexdump(s, buffer);
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
        char* buffer;
        size_t len;
        int r = getline(&buffer, &len, stdin);
        while (r!=-1) {
            process(data, buffer);
            r = getline(&buffer, &len, stdin);
        }
        free(buffer);
    }

    pn_data_free(data);

    return 0;
}

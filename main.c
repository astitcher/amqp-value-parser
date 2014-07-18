#include <proton/codec.h>
#include <proton/engine.h>

#include <stdio.h>

extern int pn_data_parse(pn_data_t* data, const char* s);

void hexdump(size_t size, char* buffer)
{
    char* c;
    for (c=buffer; c<buffer+size; c++) {
        printf("%02hhx", *c);
    }
    printf("\n");
}

int main(int argc, const char* argv[])
{
    pn_data_t* data = pn_data(16);

    int i;
    for (i=1; i<argc; ++i) {
        pn_data_clear(data);

        int r = pn_data_parse(data, argv[i]);

        if (r==0) {
            pn_data_print(data);
            printf("\n");

            /* pn_data_dump() has bad bug until 0.8 with complex types */
            /*pn_data_dump(data); */

            char buffer[1024];
            int s = pn_data_encode(data, buffer, 1024);

            printf("Encoded: %d bytes:\n", s);
            hexdump(s, buffer);
        } else {
            printf("Failed: %s\n", pn_error_text(pn_data_error(data)));
        }
    }
    pn_data_free(data);

    return 0;
}

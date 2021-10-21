#include <proton/codec.h>

#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

extern int pn_data_parse(pn_data_t* data, size_t len, const char* s);
extern int pn_data_parse_string(pn_data_t* data, const char* s);
extern size_t pn_value_dump(pn_bytes_t frame, pn_string_t *output);

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
    bool binary = false;
    bool hex = false;
    pn_data_clear(data);
    pn_error_clear(pn_data_error(data));

    // Check for special commands
    if (*str=='\\'){
        ++str;
        switch (*str) {
          case 'b':
            // Binary
            binary = true;
            break;
          case 'x':
          case 'h':
            hex = true;
            break;
          default:
            printf("Unrecognized special command: %c\n", *str);
            return;
        }
        ++str;
    }

    char buffer[1024];
    ssize_t s = 0;
    if (hex) {
      char hex[3] = {0,0,0};
      uint8_t hc = 0;
      for (const char* c=str; *c; ++c) {
        if (isspace(*c)) continue;
        if (!isxdigit(*c)) {
          printf("Illegal hex char: %c\n", *c);
          return;
        }
        hex[hc++] = *c;
        if (hc==2) {
          buffer[s++] = strtoul(hex, NULL, 16);
          hc = 0;
        }
      }
    } else {
        int r = pn_data_parse_string(data, str);
        if (r!=0) {
          printf("Failed: %s\n", pn_error_text(pn_data_error(data)));
          return;
        }
        pn_data_print(data);
        printf("\n");

        if (binary) {
            // In this mode we assume the single value is a binary item that is the bytes to be decoded
            pn_bytes_t bytes = pn_data_get_bytes(data);
            s = bytes.size;
            memcpy(buffer, bytes.start, s);
        } else {
            s = pn_data_encode(data, buffer, sizeof(buffer));
        }
    }
    printf("Encoded: %zd bytes:\n", s);
    hexdump(s, buffer);

    pn_bytes_t bytes = {.size=s,.start=buffer};
    while (bytes.size>0) {
      pn_string_t* out = pn_string("");
      size_t ds = pn_value_dump(bytes, out);
      printf("%s\n", pn_string_get(out));
      printf("(Decoded %zd bytes)\n", ds);

      bytes.size  -= ds;
      bytes.start += ds;
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

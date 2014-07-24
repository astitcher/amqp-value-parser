#include <proton/codec.h>
#include <proton/engine.h>

#include <stdio.h>
#include <stdlib.h>

extern int pn_data_parse(pn_data_t* data, size_t len, const char* s);
extern int pn_data_parse_string(pn_data_t* data, const char* s);

void hexdump(size_t size, const char* buffer)
{
    const char* c;
    for (c=buffer; c<buffer+size; c++) {
        printf("%02hhx", *c);
    }
    printf("\n");
}

void pump_collector(pn_collector_t* collector)
{
    pn_event_t* e = pn_collector_peek(collector);
    while (e) {
        printf("Event: %s\n", pn_event_type_name(pn_event_type(e)));
        pn_collector_pop(collector);
        e = pn_collector_peek(collector);
    };
}

void process(pn_transport_t* transport, pn_collector_t* collector, pn_data_t* data, const char* str)
{
    pn_data_clear(data);
    pn_error_clear(pn_data_error(data));

    int r = pn_data_parse_string(data, str);

    if (r==0) {
        pn_data_print(data);
        printf("\n");

        char buffer[1024];
        ssize_t s = pn_data_encode(data, buffer, 1024);

        printf("Encoded: %zd bytes:\n", s);
        hexdump(s, buffer);

        pn_transport_push(transport, buffer, s);
        pump_collector(collector);
        ssize_t n = pn_transport_pending(transport);
        printf("Output: %zd bytes:\n", n);
        hexdump(n, pn_transport_head(transport));
        pn_transport_pop(transport, n);
        pump_collector(collector);
    } else {
        printf("Failed: %s\n", pn_error_text(pn_data_error(data)));
    }
}

const char amqp10header[] = "AMQP\x00\x01\x00\x00";
int main(int argc, const char* argv[])
{
    /* set up proton transport to receive data */
    pn_transport_t* transport = pn_transport();
    pn_connection_t* connection = pn_connection();
    pn_collector_t* collector = pn_collector();

    pn_transport_bind(transport, connection);
    pn_connection_collect(connection, collector);

    pn_transport_push(transport, amqp10header, sizeof(amqp10header));
    pump_collector(collector);
    ssize_t n = pn_transport_pending(transport);
    printf("Output: %zd bytes:\n", n);
    hexdump(n, pn_transport_head(transport));
    pn_transport_pop(transport, n);
    pump_collector(collector);

    pn_data_t* data = pn_data(16);

    int i;
    for (i=1; i<argc; ++i) {
        process(transport, collector, data, argv[i]);
    }

    /* If we had no arguments then read from stdin */
    if (argc==1) {
        char* buffer = 0;
        size_t len = 0;
        int r = getline(&buffer, &len, stdin);
        while (r!=-1) {
            process(transport, collector, data, buffer);
            r = getline(&buffer, &len, stdin);
        }
        free(buffer);
    }

    pn_data_free(data);

    pn_collector_free(collector);
    pn_connection_free(connection);
    pn_transport_free(transport);

    return 0;
}

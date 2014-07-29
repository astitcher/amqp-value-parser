#include <proton/codec.h>
#include <proton/engine.h>
#include <proton/version.h>

#include <stdio.h>
#include <stdlib.h>

#if PN_VERSION_MAJOR>=0
#  if PN_VERSION_MINOR==7
#    define PN_EVENTS_API 1
#  elif PN_VERION_MINOR>=8
#    define PN_EVENTS_API 2
#  endif
#endif

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

void empty_output(pn_transport_t* transport)
{
    ssize_t n = pn_transport_pending(transport);
    if (n!=-1) {
        printf("Output: %zd bytes:\n", n);
        hexdump(n, pn_transport_head(transport));
        pn_transport_pop(transport, n);
    } else {
        printf("Output EOS\n");
    }
}

#if PN_EVENTS_API>=2
void pump_collector(pn_collector_t* collector)
{
    pn_event_t* e = pn_collector_peek(collector);
    while (e) {
        printf("Event: %s\n", pn_event_type_name(pn_event_type(e)));
        switch (pn_event_type(e)) {
        case PN_CONNECTION_REMOTE_OPEN: {
            pn_connection_t* connection = pn_event_connection(e);
            pn_connection_open(connection);
            break;
        }
        case PN_CONNECTION_REMOTE_CLOSE: {
            pn_connection_t* connection = pn_event_connection(e);
            pn_connection_close(connection);
            break;
        }
        case PN_TRANSPORT: {
            pn_transport_t* transport = pn_event_transport(e);
            empty_output(transport);
            break;
        }
        default:
            break;
        }
        pn_collector_pop(collector);
        e = pn_collector_peek(collector);
    };
}
#elif PN_EVENTS_API>=1
void pump_collector(pn_collector_t* collector)
{
    pn_event_t* e = pn_collector_peek(collector);
    while (e) {
        printf("Event: %s\n", pn_event_type_name(pn_event_type(e)));
        switch (pn_event_type(e)) {
        case PN_CONNECTION_REMOTE_STATE: {
            pn_connection_t* connection = pn_event_connection(e);
            if (pn_connection_state(connection) & PN_REMOTE_ACTIVE) {
              pn_connection_open(connection);
            } else if (pn_connection_state(connection) & PN_REMOTE_CLOSED) {
              pn_connection_close(connection);
            }
            break;
        }
        case PN_TRANSPORT: {
            pn_transport_t* transport = pn_event_transport(e);
            empty_output(transport);
            break;
        }
        default:
            break;
        }
        pn_collector_pop(collector);
        e = pn_collector_peek(collector);
    };
}
#else
#error "No event support"
#endif

void send_frame(pn_transport_t* transport, pn_collector_t* collector, const char* frame, size_t len)
{
    ssize_t n = pn_transport_push(transport, frame, len);
    printf("Input: %zd bytes:\n", n);
}

ssize_t make_frame(pn_data_t* data, uint16_t channel, char* buffer, size_t len)
{
    // Encode frame
    ssize_t s = pn_data_encode(data, buffer+8, 1016)+8;

    // Write frame size
    buffer[0] = (s >> 24 ) & 0xff;
    buffer[1] = (s >> 16 ) & 0xff;
    buffer[2] = (s >>  8 ) & 0xff;
    buffer[3] =  s         & 0xff;
    // Payload Offset (*4)
    buffer[4] = 2;
    // Write frame type (AMQP)
    buffer[5] = 0;
    // Write Channel
    buffer[6] = (channel >> 8) & 0xff;
    buffer[7] =  channel       & 0xff;

    return s;
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
        ssize_t s = make_frame(data, 0, buffer, sizeof(buffer));
        printf("Encoded: %zd bytes:\n", s);
        hexdump(s, buffer);

        send_frame(transport, collector, buffer, s);
        pump_collector(collector);
        empty_output(transport);
    } else {
        printf("Failed: %s\n", pn_error_text(pn_data_error(data)));
    }
}

const char amqp10header[8] = "AMQP\x00\x01\x00\x00";
const char tls10header[8] = "AMQP\x02\x01\x00\x00";
const char sasl10header[8] = "AMQP\x03\x01\x00\x00";

int main(int argc, const char* argv[])
{
    /* set up proton transport to receive data */
    pn_transport_t* transport = pn_transport();
    pn_connection_t* connection = pn_connection();
    pn_collector_t* collector = pn_collector();

    pn_transport_bind(transport, connection);
    pn_connection_collect(connection, collector);

    pump_collector(collector);

    send_frame(transport, collector, amqp10header, sizeof(amqp10header));
    pump_collector(collector);
    empty_output(transport);

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
    pn_transport_close_tail(transport);
    empty_output(transport);

    pn_data_free(data);

    pn_collector_free(collector);
    pn_connection_free(connection);
    pn_transport_free(transport);

    return 0;
}

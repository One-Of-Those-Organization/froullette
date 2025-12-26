#pragma once
#include <stdint.h>
#include "../mongoose.h"
#include "../Shared/Room.hpp"

#define MAX_MESSAGE_STRING_SIZE 512

enum MessageType {
    NONE = 0,
    ERROR,

    GIVE_ID,
    HERE_ID,

    CREATE_ROOM,
    CONNECT_ROOM,
    HERE_ROOM,
};

struct Message {
    MessageType type;
    MessageType response;
    union {
        int Int;
        char String[MAX_MESSAGE_STRING_SIZE];
        Room *Room_obj;
        // add more
    } data;
};

static size_t print_msg(void (*out)(char, void *), void *ptr, va_list *ap) {
    Message *m = va_arg(*ap, Message *);
    if (m == NULL) return 0;
    size_t n = 0;

    n += mg_xprintf(out, ptr, "{\"%s\":%d, \"%s\":%d, \"%s\":",
                    "type", (int)m->type,
                    "res", (int)m->response,
                    "data");

    switch (m->type) {
    case CREATE_ROOM:
    case GIVE_ID:
        n += mg_xprintf(out, ptr, "null");
        break;
    case HERE_ID:
        n += mg_xprintf(out, ptr, "%d", m->data.Int);
        break;
    case CONNECT_ROOM:
    case ERROR:
           n += mg_xprintf(out, ptr, "\"%s\"", m->data.String);
           break;
    case HERE_ROOM:
        break;
    default:
        n += mg_xprintf(out, ptr, "null");
        break;
    }

    n += mg_xprintf(out, ptr, "}");
    return n;
}

// EXAMPLE USAGE: mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%M", print_msg, &m);

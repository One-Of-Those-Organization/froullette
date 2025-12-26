#pragma once
#include <stdint.h>
#include "../mongoose.h"

#define MAX_MESSAGE_STRING_SIZE 512

enum MessageType {
    NONE = 0,

    GIVE_ID,
    HERE_ID,

    CREATE_ROOM,
    ID_ROOM,
};

struct Message {
    MessageType type;
    MessageType response;
    union {
        int Int;
        char String[MAX_MESSAGE_STRING_SIZE];
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
    case GIVE_ID:
        n += mg_xprintf(out, ptr, "null");
        break;
    case HERE_ID:
        n += mg_xprintf(out, ptr, "%d", m->data.Int);
        break;
    // If you decide to use the String member:
    // case SOME_TEXT_TYPE:
        //    n += mg_xprintf(out, ptr, "%Q", m->data.String);
        //    break;
    default:
        n += mg_xprintf(out, ptr, "null");
        break;
    }

    n += mg_xprintf(out, ptr, "}");
    return n;
}

// EXAMPLE USAGE: mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%M", print_msg, &m);

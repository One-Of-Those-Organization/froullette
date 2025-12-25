#pragma once
#include <stdint.h>
#include "../mongoose.h"

enum MessageType {
    NONE = 0,
    GIVE_ID,
    HERE_ID,
};

struct Message {
    MessageType type;
    MessageType response;
    union {
        int Int;
        char String[1024];
        // add more
    } data;
};

static size_t print_msg(void (*out)(char, void *), void *ptr, va_list *ap) {
    struct Message *m = va_arg(*ap, struct Message *);
    size_t n = 0;

    n += mg_xprintf(out, ptr, "{%m:%d, %m:%d, %m:",
    "type", m->type, "res", m->response, "data");

    switch (m->type) {
    case GIVE_ID:
        n += mg_xprintf(out, ptr, "null");
        break;
    case HERE_ID:
        n += mg_xprintf(out, ptr, "%d", m->data.Int);
        break;
    // case SOME_TEXT_TYPE:
    //        n += mg_xprintf(out, ptr, "%m", m->data.String);
    //        break;
    default:
        n += mg_xprintf(out, ptr, "null");
        break;
    }

    n += mg_xprintf(out, ptr, "}");
    return n;
}

// EXAMPLE USAGE: mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%M", print_msg, &m);

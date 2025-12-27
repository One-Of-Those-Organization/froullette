#pragma once
#include <stdint.h>
#include "../mongoose.h"
#include "../Shared/Room.hpp"

// protocol:
// [MSG]
// [TYPE][len][bytes]


#define MAX_MESSAGE_BIN_SIZE 512
#define MAX_MESSAGE_STRING_SIZE 512

#define WRITE_U16(p, v) do { \
    (p)[0] = (uint8_t)((v) & 0xff); \
    (p)[1] = (uint8_t)(((v) >> 8) & 0xff); \
    (p) += 2; \
} while (0)

uint16_t read_u16(const uint8_t *p) {
    return p[0] | (p[1] << 8);
}

uint32_t read_u32(const uint8_t *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

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
    default:
        n += mg_xprintf(out, ptr, "null");
        break;
    }
    n += mg_xprintf(out, ptr, "}");
    return n;
}

enum RoomField : uint8_t {
    RF_ID      = 1,
    RF_PLAYERS = 2,
    RF_STATE   = 3
};

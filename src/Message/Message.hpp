#pragma once
#include <stdint.h>
#include "../mongoose.h"
#include "../Shared/Room.hpp"
#include "../Shared/Player.hpp"

// protocol: le
// [MSG]
// [TYPE][len][bytes]

#define MAX_MESSAGE_BIN_SIZE 512
#define MAX_MESSAGE_STRING_SIZE 512

#define WRITE_U16(p, v) do { \
    (p)[0] = (uint8_t)((v) & 0xff); \
    (p)[1] = (uint8_t)(((v) >> 8) & 0xff); \
    (p) += 2; \
} while (0)

static inline void write_u32(uint8_t **pp, uint32_t v) {
    uint8_t *p = *pp;
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
    p[2] = (uint8_t)((v >> 16) & 0xff);
    p[3] = (uint8_t)((v >> 24) & 0xff);
    *pp += 4;
}

uint16_t read_u16(const uint8_t *p) {
    return p[0] | (p[1] << 8);
}

uint32_t read_u32(const uint8_t *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

enum RoomField : uint8_t {
    RF_ID           = 1,
    RF_PLAYER_COUNT = 2,
    RF_STATE        = 3
};

enum PlayerField : uint8_t {
    PF_ID     = 1,
    PF_HEALTH = 2,
};

enum MessageType {
    NONE = 0,
    ERROR,

    GIVE_ID,
    HERE_ID,

    CREATE_ROOM,
    CONNECT_ROOM,
    HERE_ROOM,
    EXIT_ROOM, // WIP

    GAME_START,// WIP
    GAME_INFO, // WIP
};

struct Message {
    MessageType type;
    MessageType response;
    union {
        int Int;
        char String[MAX_MESSAGE_STRING_SIZE];
        Room *Room_obj;
        Player *Player_obj;
        // add more
    } data;
};

struct ParsedData {
    MessageType type;
    union {
        int Int;
        char String[MAX_MESSAGE_STRING_SIZE];
        Room *Room_obj;
        Player *Player_obj;
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

[[maybe_unused]] static size_t gen_player_net_obj(uint8_t *buffer, Player *player) {
    uint8_t *p = buffer;
    *p++ = PF_ID;
    WRITE_U16(p, sizeof(uint32_t));
    write_u32(&p, player->id);

    *p++ = PF_HEALTH;
    WRITE_U16(p, 1);
    *p++ = (uint8_t)player->health;
    return (size_t)(p - buffer);
}

[[maybe_unused]] static size_t gen_room_net_obj(uint8_t *buffer, Room *r) {
    uint8_t *p = buffer;
    uint16_t id_len = strnlen(r->id, ID_MAX_COUNT);
    *p++ = RF_ID;
    WRITE_U16(p, id_len);
    memcpy(p, r->id, id_len);
    p += id_len;

    // NOTE: Player will be sended later when the game start...
    //       maybe that was right just to send the count of the player at that room
    // gen_player_net_obj(buffer, player);

    *p++ = RF_PLAYER_COUNT;
    WRITE_U16(p, 1);
    *p++ = r->player_len;

    *p++ = RF_STATE;
    WRITE_U16(p, 1);
    *p++ = (uint8_t)r->state;

    return (size_t)(p - buffer);
}

// TODO
// NOTE: Assume the buffer will be < MAX_MESSAGE_BIN_SIZE
[[maybe_unused]] static size_t generate_network_field(Message *m, uint8_t *buffer) {
    uint8_t *p = buffer;
    *p++ = m->type;

    switch(m->type) {
    case HERE_ROOM: {
        Room *r = m->data.Room_obj;
        return gen_room_net_obj(buffer, r);
    } break;
    default:
        break;
    }
    return 0;
}

// TODO: Finish this
// NOTE: dont forget to free!
[[maybe_unused]] ParsedData parse_network_packet(uint8_t *buf, size_t len) {
    ParsedData pd = {};
    uint8_t *p = buf;
    uint8_t *end = buf + len;
    uint8_t msg_type = *p++;
    switch (msg_type) {
    case HERE_ROOM: {
        pd.type = HERE_ROOM;
        pd.data.Room_obj = new Room{};
        Room *room = pd.data.Room_obj;

        while (p < end) {
            uint8_t field = *p++;
            uint16_t flen = *(uint16_t*)p; p += 2;

            switch (field) {
            case RF_ID:
                memcpy(room->id, p, flen);
                room->id[flen] = 0;
                break;

            // case RF_PLAYERS:
                //     room.player_len = *(uint32_t*)p;
                //     break;

            case RF_STATE:
                room->state = (RoomState)*p;
                break;
            default:
                // skip
                break;
            }
            p += flen;
        }

    } break;
    default:
        break;
    }
    return pd;
}

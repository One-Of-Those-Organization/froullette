#pragma once
#include <stdint.h>
#include "../mongoose.h"
#include "../Shared/Room.hpp"
#include "../Shared/Player.hpp"

// protocol: le
// [msg_len][MSG][TYPE][len][bytes]

#define MAX_MESSAGE_BIN_SIZE 512
#define MAX_MESSAGE_STRING_SIZE 512

static inline void write_u16(uint8_t **pp, uint32_t v) {
    uint8_t *p = *pp;
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
    *pp += 2;
}

static inline void write_u32(uint8_t **pp, uint32_t v) {
    uint8_t *p = *pp;
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
    p[2] = (uint8_t)((v >> 16) & 0xff);
    p[3] = (uint8_t)((v >> 24) & 0xff);
    *pp += 4;
}

static uint16_t read_u16(const uint8_t *p) {
    return p[0] | (p[1] << 8);
}

static uint32_t read_u32(const uint8_t *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

enum NeedleField : uint8_t {
    NF_ID = 1,
    NF_TYPE = 2,
};

enum RoomField : uint8_t {
    RF_ID           = 1,
    RF_PLAYER_COUNT = 2,
    RF_STATE        = 3,
    RF_PSTATE       = 4,
};

enum PlayerField : uint8_t {
    PF_ID     = 1,
    PF_HEALTH = 2,
    PF_READY  = 3,
};

enum MessageType {
    NONE = 0,
    ERROR,
    OK,

    GIVE_ID,
    HERE_ID,

    CREATE_ROOM,
    CONNECT_ROOM,
    HERE_ROOM,
    EXIT_ROOM,

    READY,              // send back boolean to say if the user ready toggle
    GAME_START,         // for ready and stuff this stuff toggle
    GAME_TURN_UPDATE,   // send the turn update after player done GAME_PLAYER_UPDATE
    GAME_PLAYER_UPDATE, // send what player do what action they take it will need new struct def.
    GAME_PERIODIC,      // will be sended every n times for the update (is this really needed?)
    GAME_END,
};

struct Message {
    MessageType type;
    MessageType response;
    union {
        uint8_t Boolean;
        int Int;
        char String[MAX_MESSAGE_STRING_SIZE];
        Room *Room_obj;
        Player *Player_obj;
        // add more
    } data;
};

[[maybe_unused]] static size_t gen_player_net_obj(uint8_t *buffer, Player *player) {
    uint8_t *p = buffer;

    *p++ = PF_ID;
    write_u16(&p, sizeof(uint32_t));
    write_u32(&p, player->id);

    *p++ = PF_HEALTH;
    write_u16(&p, 1);
    *p++ = (uint8_t)player->health;

    *p++ = PF_READY;
    *p++ = sizeof(uint8_t);
    *p++ = (uint8_t)player->ready;
    return (size_t)(p - buffer);
}

[[maybe_unused]] static size_t gen_room_net_obj(uint8_t *buffer, Room *r) {
    uint8_t *p = buffer;
    uint16_t id_len = strnlen(r->id, ID_MAX_COUNT);

    *p++ = RF_ID;
    write_u16(&p, id_len);
    memcpy(p, r->id, id_len);
    p += id_len;

    *p++ = RF_PLAYER_COUNT;
    write_u16(&p, 1);
    *p++ = r->player_len;

    *p++ = RF_STATE;
    write_u16(&p, 1);
    *p++ = (uint8_t)r->state;

    *p++ = RF_PSTATE;
    write_u16(&p, 1);
    *p++ = (uint8_t)r->turn;

    return (size_t)(p - buffer);
}

// NOTE: Assume the buffer will be < MAX_MESSAGE_BIN_SIZE
[[maybe_unused]] static size_t generate_network_field(Message *m, uint8_t *buffer) {
    uint8_t *p = buffer;

    // Reserve space for length (u16)
    uint8_t *len_ptr = p;
    write_u16(&p, 0);

    *p++ = (uint8_t)m->type;
    *p++ = (uint8_t)m->response;
    size_t payload_len = 0;
    switch (m->type) {
    case HERE_ID: {
        write_u32(&p, m->data.Int);
        payload_len = 4;
    } break;
    case HERE_ROOM: {
        Room *r = m->data.Room_obj;
        payload_len = gen_room_net_obj(p, r);
        p += payload_len;
        break;
    }
    case READY: {
        *p++ = (uint8_t)m->data.Boolean;
        payload_len++;
    } break;
    case CONNECT_ROOM:
    case ERROR:
    case NONE:
    case OK: {
        size_t str_len = strnlen(m->data.String, MAX_MESSAGE_STRING_SIZE);
        write_u16(&p, str_len);
        memcpy(p, m->data.String, str_len);
        p += str_len;
        payload_len += 2 + str_len;
        break;
    }
    case GAME_START: { /* didnt need to send anything the server already know what to do. */ } break;
    default:
        break;
    }

    uint16_t total_len = (uint16_t)(2 + payload_len);
    write_u16(&len_ptr, total_len);
    return 2 + total_len;
}
static bool parse_one_packet(
    uint8_t *buf, size_t len,
    Message *out,
    size_t *consumed
) {
    if (len < 4) return false;

    uint16_t msg_len = read_u16(buf);
    if ((uint16_t)len < msg_len + 2) return false;

    uint8_t *p = buf + 2;
    uint8_t *end = p + msg_len;

    out->type = (MessageType)*p++;
    out->response = (MessageType)*p++;

    switch (out->type) {
    case HERE_ROOM: {
        out->data.Room_obj = new Room{};
        Room *r = out->data.Room_obj;

        while (p < end) {
            uint8_t f = *p++;
            uint16_t flen = read_u16(p); p += 2;

            switch (f) {
            case RF_ID:
                memcpy(r->id, p, flen);
                r->id[flen] = 0;
                break;
            case RF_PLAYER_COUNT:
                r->player_len = *p;
                break;
            case RF_STATE:
                r->state = (RoomState)*p;
                break;
            case RF_PSTATE:
                r->turn = (PlayerState)*p;
                break;
            }
            p += flen;
        }
        break;
    }

    case READY: {
        out->data.Boolean = (uint8_t)*p;
        p++;
    } break;
    case HERE_ID:
        out->data.Int = read_u32(p);
        p += sizeof(uint32_t);
        break;
    case CONNECT_ROOM:
    case NONE:
    case ERROR:
    case OK: {
        uint16_t len = read_u16(p);
        p += 2;
        memcpy(out->data.String, p, len);
        out->data.String[len] = '\0';
        p += len;
        break;
    }

    default:
        break;
    }

    *consumed = msg_len + 2;
    return true;
}

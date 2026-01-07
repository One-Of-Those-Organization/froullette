#pragma once

#include "Player.hpp"
#include "../Game/PlayerState.hpp"

// Hardlimit for now too lazy...
// it can be implemented using da(dynamic array) but the time is just too close.
#define MAX_ROOM_COUNT 100
#define ID_MAX_COUNT 7

enum RoomState {
    ROOM_FREE = 0,
    ROOM_ACTIVE,
    ROOM_RUNNING
};

struct Room {
    char id[ID_MAX_COUNT];
    Player *players[2];
    uint8_t player_len;
    RoomState state;
    PlayerState turn;
};

int get_room_player_empty(Room *room)
{
    for (size_t i = 0; i < 2; i++) {
        if (room->players[i] == nullptr) {
            return i;
        }
    }
    return -1;
}

#pragma once

#include "Player.hpp"

// Hardlimit for now too lazy...
// it can be implemented using da(dynamic array) but the time is just too close.
#define MAX_ROOM_COUNT 100
#define ID_MAX_COUNT 16

enum RoomState {
    ROOM_FREE = 0,
    ROOM_ACTIVE,
    ROOM_RUNNING
};

struct Room {
    char id[ID_MAX_COUNT]; // 16
    Player players[2];     // 8
    uint8_t player_len;    // 1
    RoomState state;       // 1
    // NOTE: Maybe store the game data here?
};

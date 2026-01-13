#pragma once
#include "../Game/PlayerState.hpp"

#define MAX_PLAYER_COUNT 512
#define MAX_PLAYER_HEALTH 5

#ifdef _SERVER
#include "../mongoose.h"
#endif // _SERVER

struct Player {
    uint32_t id;
    uint8_t health;
#ifdef _SERVER
    mg_connection *con;
#endif // _SERVER
    bool ready;
    PlayerState turn; // TODO: apply this to all the stuff that create player.
};

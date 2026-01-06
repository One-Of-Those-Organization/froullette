#pragma once

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
};

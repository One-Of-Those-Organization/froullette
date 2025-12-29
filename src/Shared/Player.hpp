#pragma once

#define MAX_PLAYER_COUNT 512
#define MAX_PLAYER_HEALTH 5

struct Player {
    uint32_t id;
    uint8_t health;
    mg_connection *con;
};

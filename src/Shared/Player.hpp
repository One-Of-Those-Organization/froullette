#pragma once
#include <cstdlib>
#include <cstring>
#include "../Game/PlayerState.hpp"
#include "../Shared/MinimalItems.hpp"

#define MAX_PLAYER_COUNT 512
#define MAX_PLAYER_HEALTH 5
#define PLAYER_MAX_ITEMS_COUNT 3

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
  PlayerState turn;
  MinimalItems items[PLAYER_MAX_ITEMS_COUNT];
};

static inline void player_empty_item(Player *p) {
  memset(p->items, 0, sizeof(MinimalItems) * PLAYER_MAX_ITEMS_COUNT);
}

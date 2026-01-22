#pragma once

#include <cstdint>

enum GameActionType : uint8_t {
  INJECT = 1,
  USE_ITEM,
};

struct GameAction {
  GameActionType type;
  int data;
};

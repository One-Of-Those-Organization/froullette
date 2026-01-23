#pragma once
#include <stdint.h>
#include <stdlib.h>

#ifdef _SERVER
struct _MinimalNeedle {
  int id;
  uint8_t used;
  uint8_t type;
  bool revealed;
};
#endif

#ifdef _SERVER
struct Vector2 {
  float x, y;
};
#else
#include <raylib.h>
#endif

struct MinimalNeedle {
  int id;
  uint8_t used;
  Vector2 pos;
};

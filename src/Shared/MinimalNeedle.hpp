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

struct MinimalNeedle {
  int id;
  uint8_t used;
};

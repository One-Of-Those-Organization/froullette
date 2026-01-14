#pragma once
#include <stdlib.h>
#include <stdint.h>

#ifdef _SERVER
struct _MinimalNeedle {
  int id; uint8_t used; uint8_t type;
};
#endif

struct MinimalNeedle {
  int id; uint8_t used;
};


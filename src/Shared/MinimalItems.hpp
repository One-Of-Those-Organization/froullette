#pragma once
#include <cstdint>

struct MinimalItems {
  uint64_t shared_id; // so we can map to the object version
  int type;      // use ItemType
  bool used;
};

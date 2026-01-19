#pragma once
#include <functional>

struct MinimalItems {
  int shared_id; // so we can map to the object version
  int type;      // use ItemType
#ifdef SERVER_
  std::function<void()> callback;
#endif
};

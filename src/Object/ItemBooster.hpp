#pragma once

#include "../Game/PlayerState.hpp"
#include "Items.hpp"

class ItemBooster : public Items {
public:
  ItemBooster() : Items("Booster") {};
  virtual ~ItemBooster() = default;
  virtual void callback() override {
    if (!this->pstate || this->needles_id.size() <= 0)
      return;
    // TODO: Do something here.
  }
};

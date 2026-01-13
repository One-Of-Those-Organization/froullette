#pragma once

#include "../Game/PlayerState.hpp"
#include "Items.hpp"

class ItemRevealer : public Items {
  std::vector<int> needles_id;
  PlayerState *pstate;

public:
  ItemRevealer() : Items("Revealer") {};
  virtual ~ItemRevealer() = default;
  virtual void callback() override {
    if (!this->pstate || this->needles_id.size() <= 0)
      return;
    // TODO: Do something here.
  }
};

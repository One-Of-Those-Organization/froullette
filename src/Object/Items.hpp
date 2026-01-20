#pragma once

#include "Object.hpp"
#include <functional>

enum ItemType { BOOSTER = 0 , REVEALER = 1 };

class Items : public Object {
public:
  Texture *dtext[2];
  uint64_t shared_id;
  ItemType type;
  Vector2 *curpos = nullptr;
  bool used = false;
  bool _hovered = false;
  std::function<void()> callback = nullptr;
  Items(ItemType type, int id) : Object(), shared_id(id), type(type) {};
  virtual ~Items() = default;
  void logic(float dt) override {
    (void)dt;
    if (!curpos || this->used) {
      show = false;
      return;
    } else {
      show = true;
    }
    switch (this->type) {
    case BOOSTER: { this->text = this->dtext[0]; } break;
    case REVEALER: { this->text = this->dtext[1]; } break;
    }
    if (CheckCollisionPointRec(*curpos, this->rec)) {
      this->_hovered = true;
#ifdef MOBILE
      if (IsGestureDetected(GESTURE_TAP))
        this->callback();
#else
      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        this->callback();
#endif
    } else
      this->_hovered = false;
  };
};

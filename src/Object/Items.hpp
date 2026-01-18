#pragma once

#include "Object.hpp"

enum ItemType {
  BOOSTER,
  REVEALER
};

class Items : public Object {
public:
  int shared_id;
  ItemType type;
  Vector2 *curpos;
  bool _hovered;
  std::function<void()> callback = nullptr;
  Items(ItemType type, int id) : Object(), type(type), shared_id(id) {};
  virtual ~Items() = default;
  void logic(float dt) override {
    (void)dt;
    if (!curpos)
      return;
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

#pragma once

#include "Object.hpp"
#include "../Sound/ManagedSound.hpp"
#include <functional>

enum ItemType { BOOSTER = 0 , REVEALER = 1 };

class Items : public Object {
public:
  Texture *dtext[2];
  int shared_id;
  ItemType type;
  Vector2 *curpos = nullptr;
  bool used = false;
  bool _hovered = false;
  std::function<void(Items *item)> callback = nullptr;
  ManagedSound *sound = nullptr;
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
      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        this->callback(this);
        if (this->sound) this->sound->play_sound();
      }
    } else
      this->_hovered = false;
  };
};

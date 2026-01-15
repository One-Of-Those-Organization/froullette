#pragma once

#include "Object.hpp"

class Cursor : public Object {
public:
  Vector2 *cursor = nullptr;

  Cursor() : Object() {};
  ~Cursor() = default;

  void render() override {
    if (this->text) {
      DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height), rec,
                     Vector2(0, 0), 0.0f, WHITE);
    }
  };
  void logic(float dt) override {
    (void)dt;
    if (!this->show && !this->cursor)
      return;
    this->rec.x = cursor->x;
    this->rec.y = cursor->y;

    if (this->text && (this->rec.width <= 0 && this->rec.height <= 0)) {
      this->rec.width = this->text->width;
      this->rec.height = this->text->height;
      // TODO: another web gotcha... when hiden it just broken use css to hide:
      // `cursor: none`
#ifndef __EMSCRIPTEN__
      HideCursor();
#endif
    }
  };
};

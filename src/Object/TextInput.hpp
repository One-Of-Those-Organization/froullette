#pragma once

#include "Object.hpp"
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

class TextInput : public Object {
public:
  Vector2 *curpos = nullptr;
  const char *placeholder;
  int font_size;
  int *active_id = nullptr;
  bool _hovered = false;
  int _spacing = 1;
  Font *font;
  int padding;
  Color color[4];
  std::string *buffer;

  TextInput(const char *ph) : Object(), placeholder(ph) {};
  virtual ~TextInput() = default;
  void render() override {
    if (!this->show || !font)
      return;

    Color fgcolor = _hovered ? color[3] : color[1];
    Color bgcolor = _hovered ? color[2] : color[0];
    if (this->active_id) {
      if (*this->active_id == id) {
        bgcolor = color[2];
        fgcolor = color[3];
      }
    }
    DrawRectangleRec(rec, bgcolor);

    BeginScissorMode(rec.x, rec.y, rec.width, rec.height);
    if (placeholder && buffer->empty()) {
      DrawTextPro(*font, placeholder, Vector2(rec.x + padding, rec.y + padding),
                  Vector2(0, 0), 0.0f, font_size, _spacing, fgcolor);
    } else {
      DrawTextPro(*font, buffer->c_str(),
                  Vector2(rec.x + padding, rec.y + padding), Vector2(0, 0),
                  0.0f, font_size, _spacing, fgcolor);
    }
    EndScissorMode();
  };

  void logic(float dt) override {
    (void)dt;
    if (!curpos || !this->active_id)
      return;
    if (CheckCollisionPointRec(*curpos, this->rec)) {
      this->_hovered = true;
#ifdef MOBILE
      if (IsGestureDetected(GESTURE_TAP))
#else
      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
#endif
      {
        *this->active_id = this->id;

#if defined(MOBILE) && defined(__EMSCRIPTEN__)
        // EM_ASM allows inline JavaScript.
        // We create a hidden textarea and focus it to trigger the mobile keyboard.
        EM_ASM({
            var inputId = 'raylib-hidden-input';
            var input = document.getElementById(inputId);

            if (!input) {
                input = document.createElement('textarea');
                input.id = inputId;
                input.style.position = 'absolute';
                input.style.opacity = '0';
                input.style.top = '-10000px';
                input.style.left = '-10000px';
                // Prevent zooming on focus by setting font size > 16px
                input.style.fontSize = '16px';
                document.body.appendChild(input);
            }

            input.focus();
            input.click(); // Sometimes needed for specific mobile browsers
        });
#endif
      }
    } else
      this->_hovered = false;

    if (*this->active_id == id) {
      int key = GetCharPressed();
      if (key >= 32 && key <= 126) {
        char c = (char)key;
        buffer->push_back(c);
      }
      if (IsKeyPressed(KEY_BACKSPACE) && !buffer->empty()) {
        buffer->pop_back();
      }
      if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        *this->active_id = -1;
#if defined(MOBILE) && defined(__EMSCRIPTEN__)
        // Blur the input to hide keyboard when done
        EM_ASM({
            var input = document.getElementById('raylib-hidden-input');
            if (input) input.blur();
        });
#endif
      }
    }
  }

  void calculate_rec() {
    if (placeholder) {
      rec.width = this->_get_width() + this->padding * 2;
      rec.height = this->font_size + this->padding * 2;
    }
  }

  int _get_width() {
    if (font) {
      Vector2 size = MeasureTextEx(*this->font, this->placeholder,
                                   this->font_size, this->_spacing);
      return size.x;
    }
    return -1;
  }
};

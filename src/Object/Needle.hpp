#pragma once

#include "Object.hpp"
#include "../Sound/ManagedSound.hpp"
#include <functional>
#include <queue>
#include <chrono>

enum class NeedleType : int32_t {
  NT_BLANK = 0,
  NT_LIVE = 1,
};

class Needle : public Object {
public:
  NeedleType type;
  bool _hovered = false;
  bool _dragging = false;

  bool *engine_dragging = nullptr;
  const int *engine_dragged_id = nullptr;
  std::queue<int> *dragged_qq = nullptr;

  Vector2 *curpos = nullptr;
  Vector2 offset = {};
  Rectangle max_rec = {};

  bool used = false;
  int shared_id = 0;
  bool revealed = false;

  ManagedSound *sound = nullptr;

  std::chrono::time_point<std::chrono::steady_clock> _start;
  std::chrono::milliseconds _target{5000};
  bool _timer_started = false;

  std::function<void(Needle *)> callback;

  Needle() : Object() {}
  virtual ~Needle() = default;

  void render() override {
    if (!show || used) return;

    if (text) {
      Color col = WHITE;
      if (revealed) col = GetColor(0xff1111ff);
      else if (_hovered) col = GetColor(0xf0f0f0ff);

      DrawTexturePro(*text,
                     Rectangle{0, 0, (float)text->width, (float)text->height},
                     rec, Vector2{0, 0}, 0.0f, col);
    } else {
      DrawRectangleRec(rec, _hovered ? PURPLE : PINK);
    }
  }

  void logic(float dt) override {
    (void)dt;
    if (!curpos || !engine_dragging || !engine_dragged_id || !dragged_qq)
      return;
    if (used || !show)
      return;

    if (!_timer_started && revealed) {
      _timer_started = true;
      _start = std::chrono::steady_clock::now();
    }

    if (_timer_started && revealed) {
      if (std::chrono::steady_clock::now() - _start >= _target) {
        revealed = false;
        _timer_started = false;
      }
    }

    _hovered = CheckCollisionPointRec(*curpos, rec);

    /* ---------- release drag ---------- */
#ifdef MOBILE
    if (GetTouchPointCount() == 0)
      #else
      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        #endif
        {
          _dragging = false;
          *engine_dragging = false;
        }

    /* ---------- start drag (hold) ---------- */
#ifdef MOBILE
    if (_hovered && !*engine_dragging && GetTouchPointCount() == 1)
      #else
      if (_hovered && !*engine_dragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        #endif
        {
          _dragging = true;
          *engine_dragging = true;

          if (*engine_dragged_id != id)
            dragged_qq->push(id);

          offset.x = curpos->x - rec.x;
          offset.y = curpos->y - rec.y;
        }

#ifdef MOBILE
    if (_dragging && GetTouchPointCount() == 1)
#else
      if (_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
#endif
        {
          _move_rec();
        }

#ifdef MOBILE
  // NOTE: on mobile dont do anything it handled by outside button
  if (_hovered && *engine_dragged_id != id) {
      dragged_qq->push(id);
  }
#else
  // desktop right click
  if (_hovered && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
    if (*engine_dragged_id != id)
      dragged_qq->push(id);
    else if (callback) {
      if (sound) sound->play_sound();
      callback(this);
    }
  }
#endif
  }

  void _move_rec() {
    Vector2 newpos {
      curpos->x - offset.x,
      curpos->y - offset.y
    };

    if (newpos.x < max_rec.x ||
        newpos.y < max_rec.y ||
        newpos.x + rec.width  > max_rec.x + max_rec.width ||
        newpos.y + rec.height > max_rec.y + max_rec.height)
      return;

    rec.x = newpos.x;
    rec.y = newpos.y;
  }
};

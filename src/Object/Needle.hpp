#pragma once

#include "Object.hpp"

enum class NeedleType: int32_t {
    NT_BLANK = 0,
    NT_LIVE = 1,
};

class Needle : public Object {
public:
    NeedleType type;
    bool _hovered = false;
    bool _dragging = false;
    bool *engine_dragging = nullptr;
    Vector2 *curpos = nullptr;
    Vector2 offset = {};

    Needle(): Object() {};
    virtual ~Needle() = default;
    void render() override {
        if (!this->show) return;

        if (this->text) {
            if (this->_hovered) {
                DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                               rec, Vector2(0, 0), 0.0f, GetColor(0xf0f0f0ff));
            } else {
                DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                               rec, Vector2(0, 0), 0.0f, WHITE);
            }
        } else {
            if (this->_hovered) DrawRectangleRec(this->rec, PURPLE);
            else DrawRectangleRec(this->rec, PINK);
        }
    };

    void logic(float dt) override {
        (void)dt;
        if (!curpos) return;
        if (!this->engine_dragging) return;

#ifdef MOBILE
        if (IsGestureDetected(GESTURE_NONE))  {
#else
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
#endif
            this->_dragging = false;
            *this->engine_dragging = false;
        }
        this->_hovered = CheckCollisionPointRec(*curpos, this->rec);
#ifdef MOBILE
        if (this->_hovered && !*this->engine_dragging && IsGestureDetected(GESTURE_DRAG))  {
#else
        if (this->_hovered && !*this->engine_dragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ) {
#endif
            this->_dragging = true;
            *this->engine_dragging = true;

            this->offset.x = curpos->x - this->rec.x;
            this->offset.y = curpos->y - this->rec.y;
        }
        if (this->_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            _move_rec();
        }

    };

    void _move_rec() {
        rec.x = this->curpos->x - this->offset.x;
        rec.y = this->curpos->y - this->offset.y;
    }
};

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

        if (this->_hovered) DrawRectangleRec(this->rec, RED);
        else DrawRectangleRec(this->rec, PINK);
    };
    void logic(float dt) override {
        (void)dt;
        if (!curpos) return;

        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            this->_dragging = false;
            *this->engine_dragging = false;
        }
        this->_hovered = CheckCollisionPointRec(*curpos, this->rec);
        if (this->_hovered && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !*this->engine_dragging) {
            this->_dragging = true;
            *this->engine_dragging = true;

            this->offset.x = curpos->x - this->rec.x;
            this->offset.y = curpos->y - this->rec.y;
        }
        if (this->_dragging && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            _move_rec();
        }

    };

    void _move_rec() {
        rec.x = this->curpos->x - this->offset.x;
        rec.y = this->curpos->y - this->offset.y;
    }
};

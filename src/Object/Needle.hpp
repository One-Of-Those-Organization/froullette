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
    Vector2 *curpos = nullptr;

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

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && this->_dragging) this->_move_rec();
        else this->_dragging = false;

        if (CheckCollisionPointRec(*curpos, this->rec)) {
            this->_hovered = true;
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                this->_dragging = true;
                this->_move_rec();
            }
        }
        else this->_hovered = false;
    };

    void _move_rec() {
        if (!this->_dragging) return;
        // TODO: Get the value from the current offset of the vector2 mouse and the rec itself
        // NOTE: Assume the cursor will be inside the rec already.
        Vector2 offset = {
            .x = (this->rec.x + this->rec.width) - this->curpos->x,
            .y = (this->rec.y + this->rec.height) - this->curpos->y,
        };
        this->rec.x = this->curpos->x - this->rec.width / 2.0f;
        this->rec.y = this->curpos->y - this->rec.height / 2.0f;
    }
};

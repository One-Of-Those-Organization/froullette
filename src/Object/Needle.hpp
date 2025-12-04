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
        TraceLog(LOG_INFO, "curpos pointer: %p with value: %f, %f", curpos, curpos->x, curpos->y);
        if (!curpos) return;
        if (CheckCollisionPointRec(*curpos, this->rec)) {
            this->_hovered = true;
        }
        else this->_hovered = false;
    };
};

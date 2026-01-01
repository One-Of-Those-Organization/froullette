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
    int *engine_dragged_id = nullptr;
    bool disable_update = false; // call this on the client that is not the playerstate
    Vector2 *curpos = nullptr;
    Vector2 offset = {};
    Rectangle max_rec = {};
    Rectangle _tooltip_rec = {};

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
        if (!this->engine_dragging && !this->engine_dragged_id) return;

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
            *this->engine_dragged_id = this->id;

            this->offset.x = curpos->x - this->rec.x;
            this->offset.y = curpos->y - this->rec.y;
        }
        if (this->_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            _move_rec();
        }
        // NOTE: To use the needle use the right click
        if (this->_hovered && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            TraceLog(LOG_INFO, "YES");
        }

    };

    void _move_rec() {
        Vector2 newpos = {};
        newpos.x = this->curpos->x - this->offset.x;
        newpos.y = this->curpos->y - this->offset.y;

        if (newpos.x <= this->max_rec.x ||
            newpos.y <= this->max_rec.y ||
            newpos.x >= this->max_rec.width ||
            newpos.y >= this->max_rec.height
            ) {
            this->offset.x = curpos->x - this->rec.x;
            this->offset.y = curpos->y - this->rec.y;
            return;
        }
        rec.x = newpos.x;
        rec.y = newpos.y;
    }
};

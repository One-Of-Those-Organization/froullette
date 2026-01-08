#pragma once

#include "Object.hpp"
#include <functional>

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
    Vector2 *curpos = nullptr;
    Vector2 offset = {};
    Rectangle max_rec = {};
    Rectangle _tooltip_rec = {};
    bool used = false;
    int shared_id;
    std::function<void()> callback;

    Needle(): Object() {};
    virtual ~Needle() = default;
    void render() override {
        if (!this->show || this->used) return;

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
        if (this->used || !this->show) return;

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

        #ifdef MOBILE
            if (this->_dragging && IsGestureDetected(GESTURE_DRAG)) {
        #else
            if (this->_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        #endif
            _move_rec();
        }
        #ifdef MOBILE
            if (this->_hovered && IsGestureDetected(GESTURE_DOUBLETAP)) {
        #else
            if (this->_hovered && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
        #endif
            this->used = true;
            if (this->callback) this->callback();
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

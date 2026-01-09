#pragma once

#include "Object.hpp"
#include <functional>

// Enum for Needle Types
enum class NeedleType: int32_t {
    NT_BLANK = 0,
    NT_LIVE = 1,
};

// Needle Object
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

    // Initialize on_clicked as a std::function
    std::function<void(Needle*)> on_clicked;

    // Call constructor and destructor
    Needle(): Object() {};
    virtual ~Needle() = default;

// Unused manual double tap detection
//    float last_tap_time = 0.0f;
//    const float DOUBLE_TAP_THRESHOLD = 0.3f;

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

        this->_hovered = CheckCollisionPointRec(*curpos, this->rec);

// Drag Stop Logic
#ifdef MOBILE
        if (IsGestureDetected(GESTURE_NONE)) {
#else
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
#endif
            this->_dragging = false;
            *this->engine_dragging = false;
        }

        // Check if Hovered
        this->_hovered = CheckCollisionPointRec(*curpos, this->rec);

// Drag Start Logic
#ifdef MOBILE
        if (this->_hovered && !*this->engine_dragging && IsGestureDetected(GESTURE_DRAG)) {
#else
        if (this->_hovered && !*this->engine_dragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
#endif
            this->_dragging = true;
            *this->engine_dragging = true;
            *this->engine_dragged_id = this->id;

            this->offset.x = curpos->x - this->rec.x;
            this->offset.y = curpos->y - this->rec.y;
        }

// Drag Move Logic
#ifdef MOBILE
        if (this->_dragging && IsGestureDetected(GESTURE_DRAG)) {
#else
        if (this->_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
#endif
            _move_rec();
        }

// Needles Logic
#ifdef MOBILE
        if (this->_hovered && IsGestureDetected(GESTURE_DOUBLETAP) && !this->_dragging) {
#else
        // Desktop: Pakai Klik Kanan (Released) biar aman
            if (this->_hovered && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON) && !this->_dragging) {
#endif
                // Panggil fungsi on_clicked yang sudah di-assign di Game.hpp
                if (this->on_clicked) this->on_clicked(this);
            }
    };

    void _move_rec() {
    	Vector2 newpos = {};
        newpos.x = this->curpos->x - this->offset.x;
        newpos.y = this->curpos->y - this->offset.y;

        // Don't allow moving out of bounds
        if (newpos.x <= this->max_rec.x ||
            newpos.y <= this->max_rec.y ||
            newpos.x >= this->max_rec.width ||
            newpos.y >= this->max_rec.height
            ) {
            // Reset offset if out of bounds
            this->offset.x = curpos->x - this->rec.x;
            this->offset.y = curpos->y - this->rec.y;
            return;
            }

            // Move the needle
            rec.x = newpos.x;
            rec.y = newpos.y;
	}
};

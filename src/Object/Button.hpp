#pragma once

#include "Object.hpp"
#include <string>
#include <raylib.h>

class Button : public Object {
    public:
        Vector2 *curpos = nullptr;
        std::string text;
        unsigned int text_size;
        int padding;
        Color color[4];
        void (*callback)();
        bool _hovered;
        int _spacing;

        Button() {}
        virtual ~Button() = default;

        void render() override {
            if (!show) return;
        }

        void logic(float dt) override {
            (void)dt;
            if (!curpos) return;
            if (CheckCollisionPointRec(*curpos, this->rec)) {
                this->_hovered = true;
#ifdef MOBILE
/*
Consult this guy

RLAPI void SetGesturesEnabled(unsigned int flags);      // Enable a set of gestures using flags
RLAPI bool IsGestureDetected(unsigned int gesture);     // Check if a gesture have been detected
RLAPI int GetGestureDetected(void);                     // Get latest detected gesture
RLAPI float GetGestureHoldDuration(void);               // Get gesture hold time in seconds
RLAPI Vector2 GetGestureDragVector(void);               // Get gesture drag vector
RLAPI float GetGestureDragAngle(void);                  // Get gesture drag angle
RLAPI Vector2 GetGesturePinchVector(void);              // Get gesture pinch delta
RLAPI float GetGesturePinchAngle(void);                 // Get gesture pinch angle
*/
#else
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) this->callback();
#endif
            }
            else this->_hovered = false;
        }
    int _get_width() {
        // if (mGame_ptr != nullptr) {
        //     Vector2 size =
        //         MeasureTextEx(mGame_ptr->mFont, mText.c_str(), mSize, mSpacing);
        //     return size.x;
        // }
        // return -1;
    }
};

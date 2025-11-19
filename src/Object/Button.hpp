#pragma once

#include "Object.hpp"
#include <string>
#include <functional>
#include <raylib.h>

class Button : public Object {
    public:
        Vector2 *curpos = nullptr;
        std::string text;
        int text_size;
        Font *font;
        int padding;
        Color color[4];
        std::function<void()> callback;
        bool _hovered;
        int _spacing;

        Button() {
            this->_spacing = 1;
            // int defaultFontSize = 10;
            // if (this->text_size < defaultFontSize) this->text_size = defaultFontSize;
            // this->_spacing = this->text_size / defaultFontSize;
        }
        virtual ~Button() = default;

        void render() override {
            if (!this->show || !font) return;

            Color &fgcolor = _hovered ? color[3]: color[1];
            Color &bgcolor = _hovered ? color[2]: color[0];
            rec.width = this->_get_width() + this->padding * 2;
            rec.height = this->text_size + this->padding * 2;
            DrawRectangleRec(rec, bgcolor);

            if (text != "") {
                DrawTextPro(*font, text.c_str(),
                            Vector2(rec.x + padding, rec.y + padding), Vector2(0, 0), 0.0f, text_size,
                            _spacing, fgcolor);
            }
        }

        void logic(float dt) override {
            (void)dt;
            // NOTE: Collison chenck is owned by the window or engine!
            // add new method something like "isCollidingWithCursor"
            // or add new method to convert to global var
            if (!curpos) return;
            if (CheckCollisionPointRec(*curpos, this->rec)) {
                this->_hovered = true;
#ifdef MOBILE
                if (IsGestureDetected(GESTURE_TAP)) this->callback();
#else
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) this->callback();
#endif
            }
            else this->_hovered = false;
        }

    int _get_width() {
        if (font) {
            Vector2 size =
                MeasureTextEx(
                        *this->font, this->text.c_str(),
                        this->text_size, this->_spacing
                        );
            return size.x;
        }
        return -1;
    }
};

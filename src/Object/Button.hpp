#pragma once

#include "Object.hpp"
#include <string>
#include <functional>
#include <raylib.h>

class Button : public Object {
    public:
        Vector2 *curpos = nullptr;
        std::string str;
        int str_size;
        Font *font;
        int padding;
        Color color[4];
        std::function<void()> callback;
        bool _hovered;
        int _spacing;

        Button() {
            this->_spacing = 1;
        }
        virtual ~Button() = default;

        void render() override {
            if (!this->show || !font) return;

            this->calculate_rec();
            Color &fgcolor = _hovered ? color[3]: color[1];
            Color &bgcolor = _hovered ? color[2]: color[0];
            DrawRectangleRec(rec, bgcolor);

            if (str != "" || !text) {
                DrawTextPro(*font, str.c_str(),
                            Vector2(rec.x + padding, rec.y + padding), Vector2(0, 0), 0.0f, str_size,
                            _spacing, fgcolor);
            } else {
                DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                            rec, Vector2(0, 0), 0.0f, fgcolor);
            }
        }

        void calculate_rec() {
            if (str != "" || !text) {
                rec.width = this->_get_width() + this->padding * 2;
                rec.height = this->str_size + this->padding * 2;
            }
        }

        void logic(float dt) override {
            (void)dt;
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
                        *this->font, this->str.c_str(),
                        this->str_size, this->_spacing
                        );
            return size.x;
        }
        return -1;
    }
};

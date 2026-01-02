#pragma once

#include "Object.hpp"

class TextInput: public Object {
    public:
        Vector2 *curpos = nullptr;
        const char *placeholder;
        int font_size;
        int *active_id = nullptr;
        bool _hovered = false;
        int _spacing = 1;
        Font *font;
        int padding;
        Color color[4];
        std::string *buffer;

        TextInput(const char *ph): Object(), placeholder(ph) {};
        virtual ~TextInput() = default;
        void render() override {
            if (!this->show || !font) return;

            Color &fgcolor = _hovered ? color[3]: color[1];
            Color &bgcolor = _hovered ? color[2]: color[0];
            if (this->active_id) {
                if (*this->active_id == id) {
                    bgcolor = color[2];
                    fgcolor = color[3];
                }
            }
            DrawRectangleRec(rec, bgcolor);

            BeginScissorMode(rec.x, rec.y, rec.width, rec.height);
            if (placeholder && buffer->empty()) {
                DrawTextPro(*font, placeholder,
                            Vector2(rec.x + padding, rec.y + padding), Vector2(0, 0), 0.0f, font_size,
                            _spacing, fgcolor);
            } else {
                DrawTextPro(*font, buffer->c_str(),
                            Vector2(rec.x + padding, rec.y + padding), Vector2(0, 0), 0.0f, font_size,
                            _spacing, fgcolor);
            }
            EndScissorMode();
        };

        void logic(float dt) override {
            (void)dt;
            if (!curpos || !this->active_id) return;
            if (CheckCollisionPointRec(*curpos, this->rec)) {
                this->_hovered = true;
#ifdef MOBILE
                if (IsGestureDetected(GESTURE_TAP))
#else
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
#endif
                {
                    *this->active_id = this->id;
                }
            }
            else this->_hovered = false;

            if (*this->active_id == id) {
                int key = GetCharPressed();
                if (key >= 32 && key <= 126) {
                    char c = (char)key;
                    buffer->push_back(c);
                }
                if (IsKeyPressed(KEY_BACKSPACE) && !buffer->empty()) {
                    buffer->pop_back();
                }
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                    *this->active_id = -1;
                }
            }
        }

        void calculate_rec() {
            if (placeholder) {
                rec.width = this->_get_width() + this->padding * 2;
                rec.height = this->font_size + this->padding * 2;
            }
        }

        int _get_width() {
            if (font) {
                Vector2 size =
                    MeasureTextEx(*this->font, this->placeholder, this->font_size, this->_spacing);
                return size.x;
            }
            return -1;
        }
};

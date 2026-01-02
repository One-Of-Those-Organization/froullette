#pragma once 

#include "Object.hpp"
#include "Text.hpp"
#include <string>
#include <raylib.h>
#include <functional>
#include <cctype>

class TextInput : public Object {
public:
    std::string value;
    std::string placeholder;
    Font *font = nullptr;

    int base_text_size = 12;
    int text_size = 12;
    int base_width = 150;
    int padding = 3;
    int spacing = 1;

    bool focused = false;

    Color bg_color = { 40, 40, 40, 255 };
    Color bg_focus_color = { 70, 70, 70, 255 };
    Color border_color = { 120, 120, 120, 255 };
    Color text_color = WHITE;
    Color placeholder_color = GRAY;

    std::function<bool(char)> filter;

    // TextInput() = default;
    // {
    //     rec.width = 220;
    //     rec.height = text_size + padding * 2;
    // }
    
    virtual ~TextInput() = default;

    void set_filter(std::function<bool(char)> f) {
        filter = f;
    }

    const std::string &get_text() const {
        return value;
    }

    void calculate_rec() override {
        rec.width = (int)(base_width * (text_size / (float)base_text_size));
        rec.height = text_size + padding * 2;
    }
    
    void update_using_scale(float scale, Vector2 win) override {
        // Object::update_using_scale(scale, win);
        // text_size = (int)(base_text_size * scale);
        text_size = (int)(base_text_size * scale);
        calculate_rec();
        Object::update_using_scale(scale, win);
    }

    void logic(float dt) override {
        (void)dt;
        // if (CheckCollisionPointRec(GetMousePosition(), rec)) {
        //     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        //         focused = true;
        // } else {
        //     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        //         focused = false;
        // }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            focused = CheckCollisionPointRec(GetMousePosition(), rec);
        }

        if (!focused) return;

        int key = GetCharPressed();
        while (key > 0) {
            char c = (char)key;
            if (!filter || filter(c)) value.push_back(c);
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !value.empty()) {
            value.pop_back();
        }
    }

    void render() override {
        if (!show || !font) return;
        // Color bg = focused ? bg_focus_color : bg_color;
        // DrawRectangleRec(rec, bg);
        // DrawRectangleLinesEx(rec, 2, border_color);
        // const std::string &txt = value.empty() ? placeholder : value;
        // Color tc = value.empty() ? placeholder_color : text_color;
        // DrawTextPro(*font, txt.c_str(), { rec.x + padding, rec.y + padding }, { 0, 0 }, 
        //            0.0f, text_size, spacing, tc);
        // if (focused && ((int) (GetTime() * 2) % 2 == 0)) {
        //     Vector2 sz = MeasureTextEx(*font, txt.c_str(), text_size, spacing);
        //     DrawText(
        //         "|",
        //         rec.x + padding + sz.x + 2,
        //         rec.y + padding,
        //         text_size,
        //         WHITE
        //     );
        // }
        this->calculate_rec();

        // printf("TextInput render - x: %.1f, y: %.1f, w: %.1f, h: %.1f\n",
        //        rec.x, rec.y, rec.width, rec.height);
        // DrawRectangleLinesEx(rec, 4, RED);

        DrawRectangleRec(rec, focused ? bg_focus_color : bg_color);
        DrawRectangleLinesEx(rec, 2, border_color);

        bool empty = value.empty();
        const std::string &txt = empty ? placeholder : value;
        Color tc = empty ? placeholder_color : text_color;

        Vector2 text_pos = {
            rec.x + padding,
            rec.y + (rec.height - text_size) * 0.5f
        };

        DrawTextPro(
            *font,
            txt.c_str(),
            text_pos,
            { 0, 0 },
            0.0f,
            text_size,
            spacing,
            tc
        );

        if (focused && ((int)(GetTime() * 2) % 2 == 0)) {
            Vector2 sz = MeasureTextEx(
                *font,
                value.c_str(),
                text_size,
                spacing
            );

            DrawText(
                "|",
                text_pos.x + sz.x + 2,
                text_pos.y,
                text_size,
                WHITE
            );
        }
    }
};

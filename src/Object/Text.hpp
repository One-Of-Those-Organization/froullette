#pragma once
#include "Object.hpp"
#include <string>
#include <raylib.h>

class Text : public Object {
    public:
        std::string text;
        int text_size;
        Color text_color;
        Font *font;

    Text(const std::string& content, int size, Color color, Font *font)
    : text(content), text_size(size), text_color(color), font(font){
        this->draw_in_canvas = false;
    }

    Text(){
        this->draw_in_canvas = false;
    }

    void render() override {
        if (!this->show || !this->font) return;
        DrawTextEx(*this->font, this->text.c_str(), {this->rec.x, this->rec.y}, (float)this->text_size, 1.0f, this->text_color);
    }

    Vector2 calculate_len() {
        if (!this->font) return Vector2();
        return MeasureTextEx(*this->font, this->text.c_str(), (float)this->text_size, 1.0f);
    }

    void update_using_scale(float scale, Vector2 win) override {
        // NOTE: Maybe need to resize the font size too.
        Vector2 size = calculate_len();
        rec.width  = size.x;
        rec.height = size.y;
        Object::update_using_scale(scale, win);
    }

    void logic(float dt) override {
        (void)dt;
    }
};

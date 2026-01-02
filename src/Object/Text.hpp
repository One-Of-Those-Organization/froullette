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

    void logic(float dt) override {
        (void)dt;
    }
};

#pragma once

#include "ArsEng.hpp"
#include "GameState.hpp"
#include "../Object/Button.hpp"
#include "../Object/Text.hpp"


static Button *cButton(ArsEng *engine, std::string text, int text_size,
        int padding, GameState state, Vector2 pos,
        std::function<void()> callback)
{
    auto btn = new Button();
    btn->rec = {pos.x, pos.y, 1, 1};
    btn->state = state;
    btn->text = text;
    btn->text_size = text_size;
    btn->curpos = &engine->cursor;
    btn->padding = padding;
    btn->callback = callback;
    btn->font = &engine->font;
    btn->draw_in_canvas = false;
    btn->color[0] = {GetColor(0xffffffff)};
    btn->color[1] = {GetColor(0x000000ff)};
    btn->color[2] = {GetColor(0x999999ff)};
    btn->color[3] = {GetColor(0xffffffff)};
    btn->store_rec();
    return btn;
}

static Text *cText(ArsEng *engine, GameState state,
                  std::string text, size_t text_size, Color color, Vector2 pos,
                  bool center_x = true, bool center_y = true, size_t offsetx = 0, size_t offsety = 0)
{
    Vector2 wsize = engine->window_size;
    auto text1 = new Text();
    text1->text = text;
    text1->font = &engine->font;
    text1->text_size = text_size;
    text1->text_color = color;
    text1->rec = {pos.x,pos.y,100,100};
    text1->store_rec();
    text1->is_resizable = true;
    text1->position_info.center_x = center_x;
    text1->position_info.center_y = center_y;
    text1->position_info.offset.x = offsetx;
    text1->position_info.offset.y = offsety;
    text1->update_using_scale(engine->get_scale_factor(), wsize);
    text1->state = state;
    return text1;
}

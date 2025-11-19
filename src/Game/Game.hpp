#pragma once
#include "ArsEng.hpp"
#include "../Object/Balls.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Button.hpp"

static void initTestObject(ArsEng *engine, int *z) {
    auto ball = new Balls();
    ball->rec = {10, 10, 10, 10};
    ball->engine = engine;
    ball->speed = {50, 50};
    engine->om.add_object(ball, *z++);
}

static void initInGame(ArsEng *engine, Vector2 *wsize, int *z) {
    auto desk = new Desk();
    desk->angle = {0.0f, 0.5f};
    float offset = 12;
    desk->rec = {offset, wsize->y / 2 + 5, wsize->x - offset * 2, wsize->y - 5};
    desk->state = GameState::INGAME;
    engine->om.add_object(desk, *z++);
}

static Button *createButton(std::string text, ArsEng *engine, std::function<void()> callback) {
    auto btn = new Button();
    btn->rec = {0,0,1,1};
    btn->state = GameState::MENU;
    btn->text = text;
    btn->text_size = 12;
    btn->curpos = &engine->cursor;
    btn->padding = 2;
    btn->callback = callback;
    btn->font = &engine->font;
    btn->color[0] = {GetColor(0xffffffff)};
    btn->color[1] = {GetColor(0x000000ff)};
    btn->color[2] = {GetColor(0x999999ff)};
    btn->color[3] = {GetColor(0x333333ff)};
    return btn;
}

static void initMenu(ArsEng *engine, int *z) {
    engine->om.add_object(
            createButton("Play", engine, [] () { TraceLog(LOG_INFO, "Hello!"); })
            , *z++);
}

static void gameInit(ArsEng *engine) {
    int z = 1;
    Vector2 wsize = {
        engine->canvas_size.x,
        engine->canvas_size.y,
    };

    // Load Object
    initMenu(engine, &z);
    // initInGame(engine, &wsize, &z);
}

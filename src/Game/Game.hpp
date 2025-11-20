#pragma once
#include "../Object/Balls.hpp"
#include "../Object/Button.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Text.hpp"
#include "ArsEng.hpp"

// NOTE: Later will be moved to the engine i guess...
#define apply(value) (value * scale_factor[active_factor])

static const int scale_factor[] = {
    2, // 640x480
    4, // 1280x720
    6, // 1920x1080
};
static int active_factor = 1;

static void initTestObject(ArsEng *engine, int *z) {
    auto ball = new Balls();
    ball->rec = {10, 10, 10, 10};
    ball->engine = engine;
    ball->speed = {50, 50};
    ball->state = GameState::MENU;
    engine->om.add_object(ball, *z++);
}

static void initInGame(ArsEng *engine, Vector2 *wsize, int *z) {
    auto desk = new Desk();
    desk->angle = {0.0f, 0.5f};
    float offset = 12;
    desk->rec = {offset, wsize->y / 2 + 5, wsize->x - offset * 2, wsize->y - 5};
    desk->state = GameState::INGAME;
    engine->om.add_object(desk, *z++);

    auto p2 = new Object();
    p2->rec = Rectangle();
    p2->state = GameState::INGAME;
}

static Button *createButton(ArsEng *engine, std::string text, int text_size,
                            int padding, GameState state, Vector2 pos,
                            std::function<void()> callback) {
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
    return btn;
}

static void initMenu(ArsEng *engine, Vector2 *wsize, int *z) {
    auto makeBtn = [&](const char *label, float offsetY,
                    std::function<void()> cb) {
        auto btn =
            createButton(engine, label, apply(12), apply(5), GameState::MENU,
                         {wsize->x / 2, wsize->y / 2 + offsetY}, cb);

        btn->calculate_rec();

        btn->rec.x = (wsize->x - btn->rec.width) / 2.f;
        btn->rec.y = (wsize->y - btn->rec.height) / 2.f + offsetY;

        engine->om.add_object(btn, (*z)++);
    };

    auto makeTxt = [&](const char *label, float offsetY) {
        const int font_size = 24;
        auto txt = new Text(label, apply(font_size), WHITE,
                            &engine->font);
        txt->draw_in_canvas = false;
        txt->rec.x = (wsize->x - txt->calculate_len().x) / 2.f;
        txt->rec.y = apply(offsetY);
        engine->state = GameState::MENU;
        engine->om.add_object(txt, (*z)++);
    };

    makeTxt("Fate", 30);
    makeTxt("Roullete", 50);

    makeBtn("Play", 0, [engine]() {
        TraceLog(LOG_INFO, "Changing the state to `gameplay`");
        engine->state = GameState::INGAME;
    });
    makeBtn("Settings", apply(25), [engine]() {
        TraceLog(LOG_INFO, "Changing the state to `settings`");
        engine->state = GameState::SETTINGS;
    });
    makeBtn("Exit", apply(50), [engine]() {
        TraceLog(LOG_INFO, "Exiting the game");
        engine->req_close = true;
    });
}

static void gameInit(ArsEng *engine) {
    int z = 1;
    Vector2 canvas_size = {
        engine->canvas_size.x,
        engine->canvas_size.y,
    };
    Vector2 win_size = engine->window_size;

    // Load Object
    initTestObject(engine, &z);
    initMenu(engine, &win_size, &z);
    initInGame(engine, &canvas_size, &z);
}

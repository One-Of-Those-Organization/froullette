#pragma once
#include "../Object/Balls.hpp"
#include "../Object/Button.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Text.hpp"
#include "../Object/Needle.hpp"
#include "../Object/NeedleContainer.hpp"
#include "ArsEng.hpp"
#include "GameState.hpp"

#include <ctime>

struct GameData {
    size_t round_needle_count;
};

static int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void initTestObject(ArsEng *engine, int *z) {
    auto ball = new Balls();
    ball->rec = {10, 10, 10, 10};
    ball->engine = engine;
    ball->speed = {50, 50};
    ball->state = GameState::MENU | GameState::SETTINGS;
    engine->om.add_object(ball, (*z)++);
}

static void initInGame(ArsEng *engine, Vector2 *wsize, int *z) {
    auto apply = [&](int value) {
        return engine->calcf(value);
    };

    Texture2D *player2_text = engine->tm.load_texture("p2", "./assets/DoctorFix1024.png");
    auto p2 = new Object();
    p2->rec = Rectangle{ 0, 0, player2_text->width / 8, player2_text->height / 8 };
    p2->rec.x = (wsize->x - p2->rec.width) / 2;
    p2->rec.y = (wsize->y - p2->rec.height) / 2;

    p2->is_resizable = true;
    p2->position_info.use_relative = true;
    p2->position_info.center_x = true;
    p2->position_info.center_y = true;

    p2->state = GameState::INGAME;
    p2->color = WHITE;
    p2->text = player2_text;
    engine->om.add_object(p2, (*z)++);

    auto desk = new Desk();
    desk->angle = {0.0f, 0.5f};
    float offset = 16;
    desk->rec = {offset, wsize->y / 2 + 5, wsize->x - offset * 2, wsize->y - 5};

    desk->is_resizable = true;
    desk->position_info.use_relative = true;
    desk->position_info.relative_x = offset / wsize->x;
    desk->position_info.relative_y = 0.5f;
    desk->position_info.offset.y = 5;

    desk->state = GameState::INGAME;
    engine->om.add_object(desk, (*z)++);

    auto ns = new NeedleContainer(&engine->om);
    engine->om.add_object(ns, (*z)++);

    Texture2D *needle_text = engine->tm.load_texture("needle", "./assets/needle_normal.png");
    const Rectangle needle_pos = {
        .x      = desk->rec.x,
        .y      = desk->rec.y + (desk->rec.y / 6.0f),
        .width  = desk->rec.width,
        .height = desk->rec.height - (desk->rec.height / 6.0f),
    };

    ns->rec = needle_pos;
    ns->color = GetColor(0xf0f0f0ff);
    ns->state = GameState::INGAME;

    srand(time(0));
    GameData *gd = (GameData *)engine->additional_data;
    for (size_t i = 0; i < gd->round_needle_count; i++) {
        auto needle = new Needle();
        const int padding = 10;

        Rectangle current_pos = {
            .x      = padding + needle_pos.x + (i * 6),
            .y      = needle_pos.y,
            .width  = 10,
            .height = 20,
        };

        needle->max_rec = needle_pos;
        needle->text = needle_text;
        needle->engine_dragging = &engine->dragging;
        needle->rec = current_pos;
        needle->curpos = &engine->canvas_cursor;
        needle->type = rand_range(0,1) == 1 ? NeedleType::NT_LIVE : NeedleType::NT_BLANK;
        needle->state = GameState::INGAME;
        int id = engine->om.add_object(needle, (*z)++);
        ns->needles.push_back(id);
    }
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

static Button *create_resizable_button(ArsEng *engine, std::string text, int text_size,
        int padding, GameState state, float relative_x, float relative_y, float offset_y,
        std::function<void()> callback)
{
    Vector2 wsize = engine->window_size;
    auto btn = createButton(engine, text, text_size, padding, state,
            {wsize.x * relative_x, wsize.y * relative_y}, callback);

    btn->is_resizable = true;
    btn->position_info.use_relative = true;
    btn->position_info.relative_x = relative_x;
    btn->position_info.relative_y = relative_y;
    btn->position_info.center_x = true;
    btn->position_info.offset.y = offset_y;

    btn->calculate_rec();
    btn->rec.x = (wsize.x - btn->rec.width) / 2.f;
    btn->rec.y = (wsize.y - btn->rec.height) / 2.f + offset_y;

    return btn;
}

static Text *create_resizable_text(ArsEng *engine, const char *label, int font_size,
        Color color, GameState state, float offset_y) {
    Vector2 wsize = engine->window_size;
    auto txt = new Text(label, font_size, color, &engine->font);
    txt->draw_in_canvas = false;
    txt->rec.x = (wsize.x - txt->calculate_len().x) / 2.f;
    txt->rec.y = offset_y;
    txt->state = state;

    txt->is_resizable = true;
    txt->position_info.use_relative = true;
    txt->position_info.center_x = true;
    txt->position_info.relative_y = offset_y / wsize.y;

    return txt;
}

static void initMenu(ArsEng *engine, Vector2 *wsize, int *z) {
    (void) wsize;
    auto apply = [&](int value) {
        return engine->calcf(value);
    };

    auto makeBtn = [&](const char *label, float offsetY,
            std::function<void()> cb) {
        auto btn = create_resizable_button(engine, label, apply(12), apply(5),
                GameState::MENU, 0.5f, 0.5f, offsetY, cb);
        engine->om.add_object(btn, (*z)++);
    };

    auto makeTxt = [&](const char *label, float offsetY) {
        const int font_size = apply(24);
        auto txt = create_resizable_text(engine, label, font_size, WHITE,
                GameState::MENU, apply(offsetY));
        engine->om.add_object(txt, (*z)++);
    };

    makeTxt("Fate", apply(10));
    makeTxt("Roullete", apply(17));

    makeBtn("Play", 0, [engine]() { engine->request_change_state(GameState::INGAME); });
    makeBtn("Settings", apply(25), [engine]() { engine->request_change_state(GameState::SETTINGS); });
    makeBtn("Exit", apply(50), [engine]() { engine->req_close = true; });
}

static void initSettings(ArsEng *engine, Vector2 *wsize, int *z) {
    auto apply = [&](int value) {
        return engine->calcf(value);
    };

    auto makeBtn = [&](const char *label, float offsetY,
            std::function<void()> cb) {
        auto btn = create_resizable_button(engine, label, apply(12), apply(5),
                GameState::SETTINGS, 0.5f, 0.5f, offsetY, cb);
        engine->om.add_object(btn, (*z)++);
    };

    auto makeTxt = [&](const char *label, float offsetY) {
        const int font_size = apply(24);
        auto txt = create_resizable_text(engine, label, font_size, WHITE,
                GameState::SETTINGS, apply(offsetY));
        engine->om.add_object(txt, (*z)++);
    };

    auto btnhd = new Button();
    btnhd->text = "720p";
    btnhd->text_size = apply(12);
    btnhd->font = &engine->font;

    makeTxt("Settings", apply(10));
    makeBtn("1080p", apply(25), [engine]() {
        engine->request_resize(Vector2{1920, 1080});
    });

    makeBtn("Back to menu", apply(50), [engine]() { engine->request_change_state(GameState::MENU); });
}


static void gameInit(ArsEng *engine) {
    GameData *gd = new GameData();
    gd->round_needle_count = 5;
    engine->additional_data = (void *)gd;
    int z = 1;
    Vector2 canvas_size = {
        engine->canvas_size.x,
        engine->canvas_size.y,
    };
    Vector2 win_size = engine->window_size;

    // Load Object
    initTestObject(engine, &z);
    initMenu(engine, &win_size, &z);
    initSettings(engine, &win_size, &z);
    initInGame(engine, &canvas_size, &z);
}

static void gameDeinit(ArsEng *engine) {
    delete (GameData *)engine->additional_data;
}

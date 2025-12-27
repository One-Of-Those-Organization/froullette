// NOTE: Future work or rewrite please use `clay` layouting lib to make it easier
//       and pleasant the current API is SO SAD... and didnt work fully.
// WORK: Will be straigt up working for the gameplay right now
//       The server too
#pragma once
#include "../Object/Balls.hpp"
#include "../Object/Button.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Text.hpp"
#include "../Object/Needle.hpp"
#include "../Object/KeyHandler.hpp"
#include "../Object/NeedleContainer.hpp"
#include "ArsEng.hpp"
#include "GameState.hpp"
#include "PlayerState.hpp"
// #include "PlayMenu.hpp"
#include "helper.hpp"

#include <ctime>

struct GameData {
    size_t round_needle_count;
    PlayerState pstate;
};

static int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void initTestObject(ArsEng *engine, int kh_id, int *z) {
    (void)kh_id;
    auto ball = new Balls();
    ball->rec = {10, 10, 10, 10};
    ball->engine = engine;
    ball->speed = {50, 50};
    ball->state = GameState::MENU | GameState::SETTINGS;
    engine->om.add_object(ball, (*z)++);
}

static void initInGame(ArsEng *engine, int kh_id, Vector2 *wsize, int *z) {
    GameState state = GameState::INGAME;
    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    if (!kh) TraceLog(LOG_INFO, "Failed to register keybinding to the ingame state");
    else {
        kh->add_new(KEY_Q, state, [engine]() { engine->revert_state(); });
    }

    Texture2D *player2_text = engine->tm.load_texture("p2", "./assets/DoctorFix1024.png");
    auto p2 = new Object();
    p2->rec = Rectangle{ 0, 0, player2_text->width / 8.0f, player2_text->height / 8.0f };
    p2->rec.x = (wsize->x - p2->rec.width) / 2;
    p2->rec.y = (wsize->y - p2->rec.height) / 2;

    p2->state = state;
    p2->color = WHITE;
    p2->text = player2_text;
    engine->om.add_object(p2, (*z)++);

    auto desk = new Desk();
    desk->angle = {0.0f, 0.5f};
    float offset = 16;
    desk->rec = {offset, wsize->y / 2 + 5, wsize->x - offset * 2, wsize->y - 5};
    desk->color = PINK;

    desk->state = state;
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
    ns->color = GetColor(0xf0f0f055);
    ns->state = state;

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
        needle->state = state;
        engine->om.add_object(needle, (*z)++);
        ns->needles.push_back(needle);
    }
}

static void initMenu(ArsEng *engine, int kh_id, Vector2 *wsize, int *z) {
    (void)kh_id;
    GameState state = GameState::MENU;
    size_t title_size = 64;
    Color title_color = WHITE;

    Text *title1 = cText(engine, state, "Fate", title_size, title_color, {0,0});
    title1->position_info.offset.y = -title1->rec.height;
    title1->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(title1, (*z)++);

    Text *title2 = cText(engine, state, "Roullete", title_size, title_color, {0,0});
    engine->om.add_object(title2, (*z)++);


    size_t text_size = 36;
    size_t padding = 20;

    Button *btn1 = cButton(engine, "Play", text_size, padding, state, {0,0},
                           [engine]() { engine->request_change_state(GameState::PLAYMENU); }
    );
    btn1->is_resizable = true;
    btn1->position_info.center_x = true;
    btn1->position_info.center_y = true;
    btn1->calculate_rec();
    btn1->position_info.offset.y = title_size * 3;
    btn1->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn1, (*z)++);

    Button *btn2 = cButton(engine, "Conf", text_size, padding - 5, state, {0,0},
                           [engine]() { engine->request_change_state(GameState::SETTINGS); }
    );
    btn2->is_resizable = true;
    btn2->position_info.center_x = true;
    btn2->position_info.center_y = true;
    btn2->calculate_rec();
    btn2->position_info.offset.y = title_size * 3;
    btn2->position_info.offset.x = ((btn1->rec.width + btn2->rec.width) * 0.5f) + padding;
    btn2->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn2, (*z)++);

    Button *btn3 = cButton(engine, "Exit", text_size, padding - 5, state, {0,0},
                           [engine]() { engine->req_close = true; }
    );
    btn3->is_resizable = true;
    btn3->position_info.center_x = true;
    btn3->position_info.center_y = true;
    btn3->calculate_rec();
    btn3->position_info.offset.y = title_size * 3;
    btn3->position_info.offset.x = -((btn1->rec.width + btn3->rec.width) * 0.5f) - padding;
    btn3->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn3, (*z)++);
}

static void initPlayMenu(ArsEng *engine, int kh_id, Vector2 *wsize, int *z) {
    (void)kh_id;
    GameState state = GameState::PLAYMENU;
    size_t text_size = 32;
    size_t padding = 20;
    float spacing = 25.0f;

    // Button Back
    Button *btnBack = cButton(
        engine, "Back", text_size - 8, padding - 10, state, {0, 0},
        [engine]() { engine->request_change_state(GameState::MENU); }
    );
    btnBack->is_resizable = true;
    btnBack->position_info.anchor_left = true;
    btnBack->position_info.anchor_top = true;
    btnBack->position_info.offset_= { 30, 30 };
    btnBack->calculate_rec();
    btnBack->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btnBack, (*z)++);

    // Input IP port
    TextInput *ipInput = cTextInput(
        engine, "input ip port", text_size, padding + 10, state, {0,0}
    );
    inInput->position_info.center_x = true;
    inInput->position_info.center_y = true;
    inInput->calculate_rec();
    ipInput->position_info.offset.y = -120;
    ipInput->update_using_scale(engine->get_scale_factor(), *wsize);
    // filter number
    ipInput->set_char_filter([](char c) {
        return (c >= '0' && c <= '9') || c == '.' || c == ':';
    });
    engine->om.add_object(ipInput, (*z)++);

    // input room code
    TextInput *roomInput = cTextInput(
        engine, "Input Room Code", text_size, padding + 10, state, {0,0}
    );
    roomInput->position_info.center_x = true;
    roomInput->position_info.center_y = true;
    roomInput->calculate_rec();
    roomInput->position_info.offset.y = ipInput->position_info.offset.y + inInput->rec.height + spacing;
    roomInput->update_using_scale(engine->get_scale_factor(), *wsize);
    // filter alphanumeric
    roomInput->set_char_filter([](char c) {
        return isalnum(static_cast<unsigned char>(c));
    });
    engine->om.add_object(roomInput, (*z)++);

    // Join Button
    Button *btnJoin = cButton(
        engine, "join room", text_size, padding, state, {0,0},
        [engine, ipInput, roomInput]() {
            const std::string &ip = ipInput->get_text();
            const std::string &room = roomInput->get_text();
            // Logic to join the room using ip and room code
        }
    );
    btnJoin->is_resizeable = true;
    btnJoin->position_info.center_x = true;
    btnJoin->position_info.center_y = true;
    btnJoin->calculate_rec();
    btnJoin->position_info.offset.y = roomInput->position_info.offset.y + roomInput->rec.height + spacing;
    btnJoin->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btnJoin, (*z)++);

    // Create Room Button
    Button *btnCreateRoom = cButton(
        engine, "create room", text_size, padding, state, {0,0},
        []() {
            // Logic to create a new room
        }
    );
    btnCreateRoom->is_resizable = true;
    btnCreateRoom->position_info.center_x = true;
    btnCreateRoom->position_info.center_y = true;
    btnCreateRoom->calculate_rec();
    btnCreateRoom->position_info.offset.y = btnJoin->position_info.offset.y + btnJoin->rec.height + spacing;
    btnCreateRoom->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btnCreateRoom, (*z)++);

    // (void)kh_id;
    // GameState state = GameState::PLAYMENU;
    // size_t title_size = 64;
    // Color title_color = WHITE;

    // Text *title1 = cText(engine, state, "Fate", title_size, title_color, {0,0});
    // title1->position_info.offset.y = -title1->rec.height;
    // title1->update_using_scale(engine->get_scale_factor(), *wsize);
    // engine->om.add_object(title1, (*z)++);

    // Text *title2 = cText(engine, state, "Roullete", title_size, title_color, {0,0});
    // engine->om.add_object(title2, (*z)++);


    // size_t text_size = 36;
    // size_t padding = 20;

    // Button *btn1 = cButton(engine, "Play", text_size, padding, state, {0,0},
    //                        [engine]() { engine->request_change_state(GameState::PLAYMENU); }
    // );
    // btn1->is_resizable = true;
    // btn1->position_info.center_x = true;
    // btn1->position_info.center_y = true;
    // btn1->calculate_rec();
    // btn1->position_info.offset.y = title_size * 3;
    // btn1->update_using_scale(engine->get_scale_factor(), *wsize);
    // engine->om.add_object(btn1, (*z)++);

    // Button *btn2 = cButton(engine, "Conf", text_size, padding - 5, state, {0,0},
    //                        [engine]() { engine->request_change_state(GameState::SETTINGS); }
    // );
    // btn2->is_resizable = true;
    // btn2->position_info.center_x = true;
    // btn2->position_info.center_y = true;
    // btn2->calculate_rec();
    // btn2->position_info.offset.y = title_size * 3;
    // btn2->position_info.offset.x = ((btn1->rec.width + btn2->rec.width) * 0.5f) + padding;
    // btn2->update_using_scale(engine->get_scale_factor(), *wsize);
    // engine->om.add_object(btn2, (*z)++);

    // Button *btn3 = cButton(engine, "Exit", text_size, padding - 5, state, {0,0},
    //                        [engine]() { engine->req_close = true; }
    // );
    // btn3->is_resizable = true;
    // btn3->position_info.center_x = true;
    // btn3->position_info.center_y = true;
    // btn3->calculate_rec();
    // btn3->position_info.offset.y = title_size * 3;
    // btn3->position_info.offset.x = -((btn1->rec.width + btn3->rec.width) * 0.5f) - padding;
    // btn3->update_using_scale(engine->get_scale_factor(), *wsize);
    // engine->om.add_object(btn3, (*z)++);
}

static void initSettings(ArsEng *engine, int kh_id, Vector2 *wsize, int *z) {
    GameState state = GameState::SETTINGS;
    size_t title_size = 64;
    Color title_color = WHITE;

    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    if (!kh) TraceLog(LOG_INFO, "Failed to register keybinding to the settings state");
    else {
        kh->add_new(KEY_Q, state, [engine]() { engine->revert_state(); });
    }

    Text *title1 = cText(engine, state, "Game Settings", title_size, title_color, {0,0}, false, false, 10, 10);
    engine->om.add_object(title1, (*z)++);


    size_t text_size = 36;
    size_t padding = 20;

    // Button *btn1 = cButton(engine, "720p", text_size, padding - 5, state, {0,0},
    //                        [engine]() { engine->request_resize({1280, 720}); }
    // );
    // btn1->is_resizable = true;
    // btn1->position_info.center_x = true;
    // btn1->position_info.center_y = true;
    // btn1->calculate_rec();
    // btn1->update_using_scale(engine->get_scale_factor(), *wsize);
    // engine->om.add_object(btn1, (*z)++);

    Button *btn1 = cButton(engine, "1080p", text_size, padding - 5, state, {0,0},
                           [engine]() { engine->request_resize({1920, 1080}); }
    );
    btn1->is_resizable = true;
    btn1->calculate_rec();
    btn1->position_info.center_y = true;
    btn1->position_info.offset.y = -btn1->rec.height * 3;
    btn1->position_info.offset.x = padding;
    btn1->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn1, (*z)++);

    Button *btn2 = cButton(engine, "720p", text_size, padding - 5, state, {0,0},
                           [engine]() { engine->request_resize({1280, 720}); }
    );
    btn2->is_resizable = true;
    btn2->calculate_rec();
    btn2->position_info.center_y = true;
    btn2->position_info.offset_times_scale[0] = false;
    btn2->position_info.offset.y = -btn2->rec.height * 3;
    btn2->position_info.offset.x = btn1->rec.width * 2 + padding;
    btn2->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn2, (*z)++);

    /*
    Button *btn2 = cButton(engine, "1080p", text_size, padding - 5, state, {0,0},
                           [engine]() { engine->request_resize({1920, 1080}); }
    );
    btn2->is_resizable = true;
    btn2->position_info.center_x = true;
    btn2->position_info.center_y = true;
    btn2->calculate_rec();
    btn2->position_info.offset.x = -((btn1->rec.width + btn2->rec.width) * 0.5f) - padding;
    btn2->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn2, (*z)++);

    Button *btn3 = cButton(engine, "Fullscreen", text_size, padding - 5, state, {0,0},
                           []() { TraceLog(LOG_INFO, "Not implemented for now"); }
    );
    btn3->is_resizable = true;
    btn3->position_info.center_x = true;
    btn3->position_info.center_y = true;
    btn3->calculate_rec();
    btn3->position_info.offset.x = ((btn1->rec.width + btn3->rec.width) * 0.5f) + padding;
    btn3->update_using_scale(engine->get_scale_factor(), *wsize);
    engine->om.add_object(btn3, (*z)++);
    */
}


static void gameInit(ArsEng *engine) {
    GameData *gd = new GameData();
    gd->round_needle_count = 5;
    gd->pstate = PlayerState::PLAYER1;

    engine->additional_data = (void *)gd;
    int z = 1;
    Vector2 canvas_size = {
        engine->canvas_size.x,
        engine->canvas_size.y,
    };
    Vector2 win_size = engine->window_size;

    auto kh = new KeyHandler();
    kh->engine_state = &engine->state;
    int kh_id = engine->om.add_object(kh, z++);

    // Load Object
    initTestObject (engine, kh_id, &z);
    initMenu       (engine, kh_id, &win_size, &z);
    initSettings   (engine, kh_id, &win_size, &z);
    initPlayMenu   (engine, kh_id, &canvas_size, &z);
    initInGame     (engine, kh_id, &canvas_size, &z);
}

static void gameDeinit(ArsEng *engine) {
    delete (GameData *)engine->additional_data;
}

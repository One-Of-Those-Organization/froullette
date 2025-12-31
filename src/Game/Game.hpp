// NOTE: Future work or rewrite please use `clay` layouting lib to make it easier
//       and pleasant the current API is SO SAD... and didnt work fully.
#pragma once
#include "../Object/Balls.hpp"
#include "../Object/Button.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Text.hpp"
#include "../Object/Needle.hpp"
#include "../Object/KeyHandler.hpp"
#include "../Object/NeedleContainer.hpp"
#include "../Message/Message.hpp"
#include "../Shared/Room.hpp"
#include "ArsEng.hpp"
#include "GameState.hpp"
#include "PlayerState.hpp"
#include "Client.hpp"
#include <ctime>
#include <thread>

struct GameData {
    size_t round_needle_count;
    PlayerState pstate;
    Client *client;
#ifndef __EMSCRIPTEN__
    std::thread _net;
#endif
    Room *room;
};

// TODO: Finish this
// NOTE: This is handler for the native version the web version wont be using this function
static void client_handler(mg_connection *c, int ev, void *ev_data)
{
    GameData *gd = (GameData *)c->fn_data;
    if (!gd) {
        TraceLog(LOG_INFO, "Failed to get the gamedata on the network thread");
        return;
    }
    Client *client = gd->client;
    switch (ev) {
    case MG_EV_WS_OPEN: {
        if (!client) return;
        Message msg = {};
        msg.type = MessageType::GIVE_ID;
        msg.response = MessageType::NONE;
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%M", print_msg, &msg);
    } break;
    case MG_EV_WS_MSG: {
        mg_ws_message *wm = (mg_ws_message *)ev_data;
        struct mg_str payload = wm->data;
        double msgtype;
        bool success = mg_json_get_num(payload, "$.type", &msgtype);
        if (!success) break;

        switch ((int)msgtype) {
        case HERE_ID: {
            double player_id;
            success = mg_json_get_num(payload, "$.data", &player_id);
            if (!success) {
                TraceLog(LOG_INFO, "Failed to get the player id from the server!");
                break; // TODO(0): Handle error better
            }
            client->p.id = (int)player_id;
        } break;
        case HERE_ROOM: {
            mg_ws_message *wm = (mg_ws_message *)ev_data;
            if ((wm->flags & 0x0f) == WEBSOCKET_OP_BINARY) {
                ParsedData pd = parse_network_packet((uint8_t *) wm->data.buf, wm->data.len);
                if (pd.type == HERE_ROOM) gd->room = pd.data.Room_obj; // NOTE: Dont forget to free them when leaving the room!
                else TraceLog(LOG_INFO, "Wrong data on HERE_ROOM message!");
            }
        } break;
        default:
            break;
        }
    } break;
    default:
        break;
    }
}

static int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

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
        needle->engine_dragged_id = &engine->dragged_obj;
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
                           // [engine]() { engine->request_change_state(GameState::PLAYMENU); }
                           [engine]() { engine->request_change_state(GameState::INGAME); }
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
    (void)wsize;
    (void)kh_id;
    (void)z;
    (void)engine;

    /*
    GameState state = GameState::SETTINGS;
    size_t title_size = 64;
    Color title_color = WHITE;
    // NOTE: will be the place where player input the room id
    */
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


// TODO: Connect the connect_room, create_room, give_id
//       use something like this: client->send(Message{});
static void gameInit(ArsEng *engine) {
    std::string ip = "127.0.0.1";
    uint16_t port = 8000;

    GameData *gd = new GameData();
    gd->round_needle_count = 5;
    gd->pstate = PlayerState::PLAYER1;

    gd->client = new Client();

    // TODO: Call when the ip and port is inserted
    gd->client->ip = ip;
    gd->client->port = port;
    gd->client->callback = client_handler;

    // TODO: Move it to other func so it can be called when ip and port inserted
    gd->client->connect((void *)gd);
#ifndef __EMSCRIPTEN__
    // Native: run network poll in a background thread
    gd->_net = std::thread([gd]() {
        if (gd && gd->client) { gd->client->loop(100); }
    });
#else
    // Web: no network thread; WebSocket is event-driven via Emscripten callbacks
    // If you need periodic work, do it in your render/update tick.
#endif

    engine->additional_data = (void *)gd;
    int z = 1;
    Vector2 canvas_size = {
        engine->canvas_size.x,
        engine->canvas_size.y,
    };
    Vector2 win_size = engine->window_size;

    KeyHandler *kh = new KeyHandler();
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
    GameData *gd = (GameData *)engine->additional_data;
    if (gd) {
        if (gd->client) { gd->client->done = true; }
#ifndef __EMSCRIPTEN__
        if (gd->_net.joinable()) { gd->_net.join(); }
#endif
        delete gd->client;
        delete gd;
    }
}

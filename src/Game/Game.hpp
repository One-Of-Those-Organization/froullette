#pragma once

#define DEBUG_ROOM
// NOTE: Future work or rewrite please use `clay` layouting lib to make it easier

#include "../Object/Balls.hpp"
#include "../Object/Button.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Text.hpp"
#include "../Object/Needle.hpp"
#include "../Object/KeyHandler.hpp"
#include "../Object/NeedleContainer.hpp"
#include "../Object/Hbox.hpp"
#include "../Object/Script.hpp"
#include "../Object/TextInput.hpp"
#include "../Object/Timer.hpp"
#include "../Message/Message.hpp"
#include "../Shared/Room.hpp"
#include "../Shared/Player.hpp"
#include "ArsEng.hpp"
#include "GameState.hpp"
#include "PlayerState.hpp"
#include "Client.hpp"
#include <ctime>
#include <thread>
#include <format>
#include <functional>

struct GameData {
#ifndef __EMSCRIPTEN__
    std::mutex mutex;
#endif
    size_t round_needle_count;
    PlayerState pstate;
    Client *client;
#ifndef __EMSCRIPTEN__
    std::thread _net;
#endif
    Room *room;

    bool game_ended = false;

    Player player;
    Player player1;
    Player player2;

    std::string url_buffer;
    std::string buffer;

    std::string *text_buffer;
    bool text_buffer_displayed;
};

// Create Debug Mode for Dummy Room
static void debug_mode(ArsEng *engine, GameData *gd) {
    #ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
    #endif

    // If Room not exist, create new
    if (!gd->room) {
        gd->room = new Room();
        gd->room->state = ROOM_RUNNING;
    }

    // Dummy Room Setup Steps:
    strcpy(gd->room->id, "DEBUG");
    gd->room->state = ROOM_RUNNING; // Update State
    gd->room->turn = PlayerState::PLAYER1; // Player 1 Start First
    gd->room->player_len = 2; // 2 Players

    // Dummy GameData Setup Steps:
    gd->player.id = 100;
    gd->player.ready = true;
    gd->pstate = PlayerState::PLAYER1;

    // Setup Player 1 and Player 2 Data
    gd->player1.id = 100;
    gd->player1.health = 4;
    gd->player2.id = 200;
    gd->player2.health = 4;

    // Input the players into the room
    gd->room->players[0] = &gd->player1;
    gd->room->players[1] = &gd->player2;

    TraceLog(LOG_INFO, "DEBUG: Dummy Room Created via 'T' Key");

    // Instant Throw to Ingame for Testing
    engine->request_change_state(GameState::INGAME);
}

// TODO: Finish this
// NOTE: This is handler for the native version the web version wont be using this function
static void client_handler(mg_connection *c, int ev, void *ev_data)
{
    GameData *gd = (GameData *)c->fn_data;
    if (!gd) {
        TraceLog(LOG_INFO, "NET: Failed to get the gamedata on the network thread");
        return;
    }
    Client *client = gd->client;
    switch (ev) {
    case MG_EV_WS_OPEN: {
        TraceLog(LOG_INFO, "NET: WebSocket handshake complete!");
        if (!client) return;
        client->on_connected();
    } break;
    case MG_EV_WS_MSG: {
        mg_ws_message *wm = (mg_ws_message *)ev_data;
        if ((wm->flags & 0x0f) != WEBSOCKET_OP_BINARY) break;
        uint8_t *buf = (uint8_t *)wm->data.buf;
        size_t len = wm->data.len;
        size_t off = 0;

        while (off < len) {
            Message pd{};
            size_t used = 0;

            if (!parse_one_packet(buf + off, len - off, &pd, &used))
            break;

            off += used;

            switch (pd.type) {
            case HERE_ID: {
#ifndef __EMSCRIPTEN__
                std::lock_guard<std::mutex> lock(gd->mutex);
#endif
                gd->player.id = pd.data.Int;
                TraceLog(LOG_INFO, "NET: assigned id %d", gd->player.id);
            } break;
            case HERE_ROOM: {
#ifndef __EMSCRIPTEN__
                std::lock_guard<std::mutex> lock(gd->mutex);
#endif
                gd->room = pd.data.Room_obj; // this allocate mem dont forget to free
                TraceLog(LOG_INFO, "NET: room id %s", gd->room->id);
            } break;
            case READY: {
#ifndef __EMSCRIPTEN__
                std::lock_guard<std::mutex> lock(gd->mutex);
#endif
                gd->player.ready = pd.data.Boolean;
            } break;
            case ERROR: {
#ifndef __EMSCRIPTEN__
                std::lock_guard<std::mutex> lock(gd->mutex);
#endif
                TraceLog(LOG_INFO, "NET: Error: %s", pd.data.String);
                gd->text_buffer_displayed = false;
                *gd->text_buffer = pd.data.String;
            } break;
            case OK:
            case NONE: {
#ifndef __EMSCRIPTEN__
                std::lock_guard<std::mutex> lock(gd->mutex);
#endif
                TraceLog(LOG_INFO, "NET: Info: %s", pd.data.String);
                gd->text_buffer_displayed = false;
                *gd->text_buffer = pd.data.String;
            } break;

            case GAME_TURN_UPDATE: {
                #ifndef __EMSCRIPTEN__
                    std::lock_guard<std::mutex> lock(gd->mutex);
                #endif
                    gd->room->turn = (PlayerState)pd.data.Int;
                    TraceLog(LOG_INFO, "NET: Turn Update received: %d", pd.data.Int);
            } break;

            case GAME_PLAYER_UPDATE: {
                #ifndef __EMSCRIPTEN__
                    std::lock_guard<std::mutex> lock(gd->mutex);
                #endif
                    gd->player.health = pd.data.Int;

                    if (gd->pstate == PlayerState::PLAYER1) {
                        gd->player1.health = pd.data.Int;
                    } else {
                        gd->player2.health = pd.data.Int;
                    }
                    TraceLog(LOG_INFO, "NET: Player Update received: %d", pd.data.Int);
            } break;

            // For the GAME_END should it be called when the game is ended?
            case GAME_END: {
                #ifndef __EMSCRIPTEN__
                    std::lock_guard<std::mutex> lock(gd->mutex);
                    gd->game_ended = true;
                #endif
                    // Handle Game Over Here for now i just throw to GAME MENU
                    TraceLog(LOG_INFO, "NET: Game Ended");
            } break;

            default:
                break;
            }
        }
    } break;
    case MG_EV_OPEN: {
        TraceLog(LOG_INFO, "NET: Connection created");
    } break;
    case MG_EV_ERROR: {
        TraceLog(LOG_ERROR, "NET: Error: %s", (char *)ev_data);
    } break;
    case MG_EV_CLOSE: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        TraceLog(LOG_INFO, "NET: Connection closed");
        if (gd->room) delete gd->room;
        gd->room = nullptr;
        gd->player = Player{};
        if (client) {
            client->on_disconnected();
        }
    } break;
    default:
        break;
    }
}

static int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static bool start_connection(ArsEng *engine) {
    GameData *gd = (GameData *)engine->additional_data;
#ifndef __EMSCRIPTEN__
                std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    if (gd->url_buffer.empty()) return false;
    gd->client->url = gd->url_buffer.c_str();
    if (!gd->client->connect((void *)gd)) {
        TraceLog(LOG_INFO, "NET: Failed to connect to the specified server");
        return false;
    }
    Message msg = {};
    msg.type = MessageType::GIVE_ID;
    msg.response = MessageType::NONE;
    gd->client->send(msg);
    return true;
}

static TextInput *cTextInput(ArsEng *engine, const char *placeholder, std::string *buffer, int text_size,
                            int padding, GameState state, Vector2 pos)
{
    TextInput *ti = new TextInput(placeholder);
    ti->font = &engine->font;
    ti->font_size = text_size;
    ti->state = state;
    ti->rec = { pos.x, pos.y, 100, 100};
    ti->curpos = &engine->bigcanvas_cursor;
    ti->padding = padding;
    ti->draw_in_canvas = false;
    ti->color[3] = {GetColor(0x000000ff)};
    ti->color[2] = {GetColor(0xffffffff)};
    ti->color[1] = {GetColor(0x000000ff)};
    ti->color[0] = {GetColor(0xccccccff)};
    ti->buffer = buffer;
    ti->active_id = &engine->active;
    ti->calculate_rec();
    return ti;
}

static Button *cButton(ArsEng *engine, std::string text, int text_size,
        int padding, GameState state, Vector2 pos,
        std::function<void()> callback)
{
    auto btn = new Button();
    btn->rec = {pos.x, pos.y, 1, 1};
    btn->state = state;
    btn->str = text;
    btn->str_size = text_size;
    btn->curpos = &engine->bigcanvas_cursor;
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

static Text *cText(ArsEng *engine, GameState state,
                  std::string text, size_t text_size, Color color, Vector2 pos)
{
    auto text1 = new Text();
    text1->text = text;
    text1->font = &engine->font;
    text1->text_size = text_size;
    text1->text_color = color;
    text1->rec = {pos.x,pos.y,100,100};
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

static void initInGame(ArsEng *engine, int kh_id, int *z) {
    Vector2 wsize = { (float)engine->canvas.texture.width, (float)engine->canvas.texture.height };
    GameState state = GameState::INGAME;
    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    GameData *gd = (GameData *)engine->additional_data;

    if (!kh) TraceLog(LOG_INFO, "Failed to register keybinding to the ingame state");
    else {
        kh->add_new(KEY_Q, state, [engine]() { engine->revert_state(); });
    }

    int text_size = 32;
    int padding = 20;
    Texture2D *exit_icon = engine->tm.get_texture("exit");
    Button *btnexit = cButton(engine, "", text_size, padding, state, {0,0},
                           [engine]() { engine->revert_state(); }
    );
    btnexit->text = exit_icon;
    btnexit->calculate_rec();
    btnexit->rec.x = padding;
    btnexit->rec.y = padding;
    btnexit->rec.width  = 64;
    btnexit->rec.height = 64;
    engine->om.add_object(btnexit, (*z)++);

    Texture2D *player2_text = engine->tm.load_texture("p2", "./assets/DoctorFix1024.png");
    auto p2 = new Object();
    p2->rec = Rectangle{ 0, 0, player2_text->width / 8.0f, player2_text->height / 8.0f };
    p2->rec.x = (wsize.x - p2->rec.width) / 2;
    p2->rec.y = (wsize.y - p2->rec.height) / 2;

    p2->state = state;
    p2->color = WHITE;
    p2->text = player2_text;
    engine->om.add_object(p2, (*z)++);

    Texture2D *desk_text = engine->tm.load_texture("desk", "./assets/desk.png");
    auto desk = new Desk();
    desk->angle = {0.0f, 0.5f};
    desk->text = desk_text;
    float offset = 16;
    desk->rec = {offset, wsize.y / 2 + 5, wsize.x - offset * 2, wsize.y - 5};
    desk->color = GetColor(0x333333ff);

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
    for (size_t i = 0; i < gd->round_needle_count; i++) {
        auto needle = new Needle();
        const int padding = 10;

        needle->on_clicked = [gd](Needle* n) {
            // Allow click if it's the player's turn and needle is not used
            if (gd->room->turn != gd->pstate) return;

            n->used = true;

            // Send Needle Action
            Message msg = {};
            msg.type = GAME_PLAYER_UPDATE;
            msg.data.Int = n->shared_id;
            gd->client->send(msg);
        };

        // TODO : Check if both players have used their needles to end the round
        // WIP (Work In Progress)

        Rectangle current_pos = {
            .x      = padding + needle_pos.x + (i * 6),
            .y      = needle_pos.y,
            .width  = 10,
            .height = 20,
        };

        needle->shared_id = i;
        needle->max_rec = needle_pos;
        needle->text = needle_text;
        needle->engine_dragging = &engine->dragging;
        needle->engine_dragged_id = &engine->dragged_obj;
        needle->rec = current_pos;
        needle->curpos = &engine->canvas_cursor;
        needle->type = rand_range(0,1) == 1 ? NeedleType::NT_LIVE : NeedleType::NT_BLANK;
        needle->state = state;
        needle->used = false;
        engine->om.add_object(needle, (*z)++);
        ns->needles.push_back(needle);
    }

    // This for single player but we need to update for 2 players
    // Text *hp_display = cText(engine, state, "HP: 4", 32, RED, {wsize.x - 100, wsize.y - 80});
    // NOTE : For now i use hardcoded values but i don't know how to make it responsive properly
    Text *hp_display = cText(engine, state, "HP: 4", 32, RED, {1100, 650});
    engine->om.add_object(hp_display, (*z)++);

    Script *hpUpdater = new Script();
    hpUpdater->state = state;

    hpUpdater->callback = [gd, hp_display]() {
        // This for single player but we need to update for 2 players
        //  hp_display->text = TextFormat("HP: %d", gd->player.health);
        //  if (gd->player.health <= 0) {
        //  hp_display->text_color = BLACK;
        //  TraceLog(LOG_INFO, "Player Died");
        //  }

        // Updated for 2 players
        if (gd->room->turn == PlayerState::PLAYER1) {
            hp_display->text = TextFormat("P1 HP: %d", gd->player1.health);
            hp_display->text_color = (gd->player1.health <= 0) ? BLACK : RED;
        } else {
            hp_display->text = TextFormat("P2 HP:  %d", gd->player2.health);
            hp_display->text_color = (gd->player2.health <= 0) ? BLACK :  BLUE;
        }

        if (gd->player.health <= 0 || gd->player.health <= 0) {
            TraceLog(LOG_INFO, "Game Over");
        }

        // TODO : Add win condition and game over handling here, check who win and create some menu restart or go to main menu
    };
    engine->om.add_object(hpUpdater, (*z)++);
}

static void initMenu(ArsEng *engine, int kh_id, int *z) {
    Vector2 wsize = { (float)engine->bigcanvas.texture.width, (float)engine->bigcanvas.texture.height };
    (void)kh_id;
    GameState state = GameState::MENU;
    size_t title_size = 64;
    Color title_color = WHITE;

    Text *title1 = cText(engine, state, "Fate", title_size, title_color, {0,0});
    Vector2 title1_len = title1->calculate_len();
    title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
    title1->rec.y = (wsize.y / 2.0f) - title1_len.y;
    engine->om.add_object(title1, (*z)++);

    Text *title2 = cText(engine, state, "Roullete", title_size, title_color, {0,0});
    Vector2 title2_len = title2->calculate_len();
    title2->rec.x = (wsize.x - title2_len.x) / 2.0f;
    title2->rec.y = (wsize.y / 2.0f);
    engine->om.add_object(title2, (*z)++);

    size_t text_size = 36;
    size_t padding = 20;

    Button *btn1 = cButton(engine, "Start", text_size, padding, state, {0,0},
                           [engine]() { engine->request_change_state(GameState::PLAYMENU); }
                           // [engine]() { engine->request_change_state(GameState::INGAME); }
    );
    btn1->calculate_rec();
    btn1->rec.x = (wsize.x - btn1->rec.width) / 2.0f;
    btn1->rec.y = wsize.y - (btn1->rec.height + padding * 5);
    engine->om.add_object(btn1, (*z)++);

    Texture2D *settings_cog = engine->tm.load_texture("cogs", "./assets/settings.png");
    Button *btn2 = cButton(engine, "", text_size, padding, state, {0,0},
                           [engine]() { engine->request_change_state(GameState::SETTINGS); }
    );
    btn2->text = settings_cog;
    btn2->calculate_rec();
    btn2->rec.x = btn1->rec.x + btn1->rec.width + padding;
    btn2->rec.y = btn1->rec.y;
    btn2->rec.width = btn1->rec.height;
    btn2->rec.height = btn1->rec.height;
    engine->om.add_object(btn2, (*z)++);

    Texture2D *exit_icon = engine->tm.load_texture("exit", "./assets/exit.png");
    Button *btn3 = cButton(engine, "", text_size, padding, state, {0,0},
                           [engine]() { engine->req_close = true; }
    );
    btn3->text = exit_icon;
    btn3->calculate_rec();
    btn3->rec.x = btn1->rec.x - (btn1->rec.height + padding);
    btn3->rec.y = btn1->rec.y;
    btn3->rec.width = btn1->rec.height;
    btn3->rec.height = btn1->rec.height;
    engine->om.add_object(btn3, (*z)++);

    // Debug Mode
    #ifdef DEBUG_ROOM
    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    if (kh) {
        // Access Game Data only for DEBUG
        GameData *gd = (GameData *)engine->additional_data;

        // Toggle DEBUG Mode
        // Make sure you were in MENU
        kh->add_new(KEY_T, GameState::MENU, [engine, gd]() {
            debug_mode(engine, gd);
        });
    }
    #endif
}

static void initPlayMenu(ArsEng *engine, int kh_id, int *z) {
    Vector2 wsize = { (float)engine->bigcanvas.texture.width, (float)engine->bigcanvas.texture.height };
    GameState state = GameState::PLAYMENU;
    int padding = 20;
    int text_size = 32;
    int title_size = 64;
    Color title_color = WHITE;

    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    if (!kh) TraceLog(LOG_INFO, "Failed to register keybinding to the playmenu state");
    else {
        // NOTE: Disable this binding because user can actually type q on the inputbox
        // kh->add_new(KEY_Q, state, [engine]() { engine->revert_state(); });
    }
    GameData *gd = (GameData *)engine->additional_data;

    Texture2D *exit_icon = engine->tm.get_texture("exit");
    if (!exit_icon) {
        TraceLog(LOG_FATAL, "Failed to get the EXIT TEXTURE!");
        return;
    }

    Text *title1 = cText(engine, state, "Get ready", title_size, title_color, {0,0});
    Vector2 title1_len = title1->calculate_len();
    title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
    title1->rec.y = title1_len.y + padding;
    engine->om.add_object(title1, (*z)++);

    TextInput *url = cTextInput(engine, "Enter ip:port", &gd->url_buffer, text_size, padding,
                               state, { wsize.x / 2.0f, title1->rec.y + title1->rec.height });
    url->rec.width = (wsize.x - padding * 5) / 2.0f;
    url->rec.x = (wsize.x - url->rec.width) / 2.0f;
    engine->om.add_object(url, (*z)++);

    TextInput *id = cTextInput(engine, "Room id (insert only on connect)", &gd->buffer, text_size, padding,
                               state, { wsize.x / 2.0f, url->rec.y + url->rec.height + (padding * 2) });
    id->rec.width = (wsize.x - padding * 5) / 2.0f;
    id->rec.x = (wsize.x - id->rec.width) / 2.0f;
    engine->om.add_object(id, (*z)++);

    HBox *hbox = new HBox();
    hbox->state = state;
    hbox->rec.x = padding * 10;
    hbox->rec.y = id->rec.y + id->rec.height + (padding * 2);
    hbox->rec.width = wsize.x - (hbox->rec.x * 2);
    hbox->rec.height = 64 + padding;
    hbox->padding = padding;
    hbox->al = Alignment::CENTER;
    hbox->draw_in_canvas = false;
    engine->om.add_object(hbox, (*z)++);

    Button *btncreate =
        cButton(engine, "Create room", text_size, padding, state, {0,0},
            [engine, gd]() {
                if (!gd->client->c) {
                    if (!start_connection(engine)) return;
                }
                Message msg = {};
                msg.type = CREATE_ROOM;
                msg.response = NONE;
                gd->client->send(msg);
        });
    btncreate->calculate_rec();
    engine->om.add_object(btncreate, (*z)++);
    hbox->add_child(btncreate);
    hbox->position_child();

    Button *btnconnect = cButton(engine, "Connect to room", text_size, padding, state, {0,0},
            [engine, gd]() {
                if (!gd->client->c) {
                    if (!start_connection(engine)) return;
                }
                Message msg = {};
                msg.type = CONNECT_ROOM;
                msg.response = NONE;
                strncpy(msg.data.String, gd->buffer.data(), MAX_MESSAGE_STRING_SIZE);
                gd->client->send(msg);
        });
    btnconnect->calculate_rec();
    engine->om.add_object(btnconnect, (*z)++);
    hbox->add_child(btnconnect);
    hbox->position_child();

    Button *btn1 = cButton(engine, "", 0, padding, state, {0,0},
                           [engine]() { engine->request_change_state(GameState::MENU); }
    );
    btn1->text = exit_icon;
    btn1->rec.width = 64;
    btn1->rec.height = 64;
    btn1->rec.x = padding;
    btn1->rec.y = padding;
    engine->om.add_object(btn1, (*z)++);
}

static void initSettings(ArsEng *engine, int kh_id, int *z) {
    Vector2 wsize = { (float)engine->bigcanvas.texture.width, (float)engine->bigcanvas.texture.height };
    GameState state = GameState::SETTINGS;
    size_t title_size = 64;

    #ifndef __EMSCRIPTEN__
    size_t text_size = 32;
    #endif
    size_t padding = 20;
    Color title_color = WHITE;

    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    if (!kh) TraceLog(LOG_INFO, "Failed to register keybinding to the settings state");
    else {
        kh->add_new(KEY_Q, state, [engine]() { engine->revert_state(); });
    }

    Text *title1 = cText(engine, state, "Settings", title_size, title_color, {0,0});
    Vector2 title1_len = title1->calculate_len();
    title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
    title1->rec.y = title1_len.y + padding;
    engine->om.add_object(title1, (*z)++);

    #ifndef __EMSCRIPTEN__
    Text *restext = cText(engine, state, "Resolution", text_size, title_color, {0,0});
    Vector2 restext_len = restext->calculate_len();
    restext->rec.x = (wsize.x - restext_len.x) / 2.0f;
    restext->rec.y = title1->rec.y + (padding * 5 + restext_len.y);
    engine->om.add_object(restext, (*z)++);

    // res here
    HBox *hbox = new HBox();
    hbox->state = state;
    hbox->rec.x = padding * 10;
    hbox->rec.y = restext->rec.y + restext_len.y + (padding * 2);
    hbox->rec.width = wsize.x - (hbox->rec.x * 2);
    hbox->rec.height = 64 + padding;
    hbox->padding = padding;
    hbox->al = Alignment::CENTER;
    hbox->draw_in_canvas = false;
    engine->om.add_object(hbox, (*z)++);

    Button *btnfull = cButton(engine, "Toggle Fullscreen", text_size, padding, state, {0,0},
                              [engine]() { engine->request_fullscreen(); }
    );
    btnfull->calculate_rec();
    engine->om.add_object(btnfull, (*z)++);
    hbox->add_child(btnfull);
    hbox->position_child();

    Button *btnhd = cButton(engine, "720p", text_size, padding, state, {0,0},
                              [engine]() { engine->request_resize({1280, 720}); }
    );
    btnhd->calculate_rec();
    engine->om.add_object(btnhd, (*z)++);
    hbox->add_child(btnhd);
    hbox->position_child();

    Button *btnfhd = cButton(engine, "1080p", text_size, padding, state, {0,0},
                              [engine]() { engine->request_resize({1920, 1080}); }
    );
    btnfhd->calculate_rec();
    engine->om.add_object(btnfhd, (*z)++);
    hbox->add_child(btnfhd);
    hbox->position_child();
    #endif // __EMSCRIPTEN__

    Texture2D *exit_icon = engine->tm.get_texture("exit");
    if (!exit_icon) {
        TraceLog(LOG_FATAL, "Failed to get the EXIT TEXTURE!");
        return;
    }
    Button *btn1 = cButton(engine, "", 0, padding, state, {0,0},
                           [engine]() { engine->revert_state(); }
    );
    btn1->text = exit_icon;
    btn1->rec.width = 64;
    btn1->rec.height = 64;
    btn1->rec.x = padding;
    btn1->rec.y = padding;
    engine->om.add_object(btn1, (*z)++);
}

static void initRoomMenu(ArsEng *engine, int kh_id, int *z) {
    Vector2 wsize = { (float)engine->bigcanvas.texture.width, (float)engine->bigcanvas.texture.height };
    GameData *gd = (GameData *)engine->additional_data;
    GameState state = GameState::ROOMMENU;
    KeyHandler *kh = (KeyHandler*)engine->om.get_object(kh_id);
    if (!kh) TraceLog(LOG_INFO, "Failed to register keybinding to the ingame state");
    else {
        kh->add_new(KEY_Q, state, [engine, gd]() {
            Message msg = {};
            msg.type = EXIT_ROOM;
            msg.response = NONE;
            gd->client->send(msg);
            if (gd->room) {
                delete gd->room;
                gd->room = nullptr; // NOTE: IDK if this is the best approach but yeah...
            }
            engine->revert_state();
        });

    }

    size_t text_size = 32;
    size_t padding = 20;
    Color title_color = WHITE;

    Text *text_id = cText(engine, state, "Room id: _", text_size, title_color, {0,0});
    Vector2 text_id_len = text_id->calculate_len();
    text_id->rec.x = (wsize.x - text_id_len.x) / 2.0f;
    text_id->rec.y = (wsize.y / 4.0f) - text_id_len.y;
    engine->om.add_object(text_id, (*z)++);

    Texture2D *exit_icon = engine->tm.get_texture("exit");
    if (!exit_icon) {
        TraceLog(LOG_FATAL, "Failed to get the EXIT TEXTURE!");
        return;
    }

    Button *btn1 = cButton(engine, "", 0, padding, state, {0,0},
                    [engine]() {
                        GameData *gd = (GameData *)engine->additional_data;
                        Message msg = {};
                        msg.type = EXIT_ROOM;
                        msg.response = NONE;
                        gd->client->send(msg);
                        if (gd->room) {
                            delete gd->room;
                            gd->room = nullptr; // NOTE: IDK if this is the best approach but yeah...
                        }
                        engine->revert_state();
                    });
    btn1->text = exit_icon;
    btn1->rec.width = 64;
    btn1->rec.height = 64;
    btn1->rec.x = padding;
    btn1->rec.y = padding;
    engine->om.add_object(btn1, (*z)++);

    Button *btn2 = cButton(engine, "Ready", text_size, padding, state, {0,0},
                [engine]() {
                        GameData *gd = (GameData *)engine->additional_data;
                        Message msg = {};
                        msg.type = GAME_START;
                        msg.response = NONE;
                        gd->client->send(msg);
                    });
    btn2->calculate_rec();
    btn2->rec.x = (wsize.x - btn2->rec.width) / 2.0f;
    btn2->rec.y = (wsize.y - btn2->rec.width) / 2.0f;
    engine->om.add_object(btn2, (*z)++);

    Script *sc = new Script();
    sc->callback = [engine, text_id, wsize, btn2, gd]() {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        if (gd->room) {
            text_id->text = std::format("Room id: {}",gd->room->id);
            text_id->rec.x = (wsize.x - text_id->calculate_len().x) / 2.0;
        }
        else if (has_flag(engine->state, GameState::ROOMMENU)){
            engine->request_change_state(GameState::PLAYMENU);
        }

        // NOTE: This will be bad for performance but i guess for simplicity and for the result of my bad design
        //       legit this is so bad...
        //  if (gd->player.ready && btn2->str != "Ready") {
        //  btn2->str = "Ready";
        //  btn2->calculate_rec();
        //  btn2->rec.x = (wsize.x - btn2->rec.width) / 2.0f;
        //  } else if (!gd->player.ready && btn2->str != "Unready"){
        //  btn2->str = "Unready";
        //  btn2->calculate_rec();
        //  btn2->rec.x = (wsize.x - btn2->rec.width) / 2.0f;
        //  }

        // I'm trying to optimize this a bit maybe it's better? I don't know
        static bool last_ready = false;
        if (gd->player.ready != last_ready) {
            last_ready = gd->player.ready;
            btn2->str = gd->player.ready ? "Unready" : "Ready";
            btn2->calculate_rec();
            btn2->rec.x = (wsize.x - btn2->rec.width) / 2.0f;
        }
    };
    engine->om.add_object(sc, (*z)++);

}

static void initALLObject(ArsEng *engine, int kh_id, int *z) {
    Vector2 wsize = { (float)engine->bigcanvas.texture.width, (float)engine->bigcanvas.texture.height };
    (void)kh_id;
    GameState state = GameState::ALL;
    Script *sc = new Script();
    sc->state = state;
    GameData *gd = (GameData *)engine->additional_data;
    sc->callback = [engine, gd]() {
        if (gd->game_ended) {
            gd->game_ended = false;
            engine->request_change_state(GameState::MENU);
            return;
        }
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        if (gd->room && !has_flag(engine->state, GameState::ROOMMENU | GameState::INGAME | GameState::FINISHED))
        {
            GameState target = GameState::ROOMMENU;
            if (gd->room->state == ROOM_RUNNING) target = GameState::INGAME;
            engine->request_change_state(target);
            return;
        }
#ifndef DEBUG_ROOM
        if (!gd->room && gd->player.id == 0 && has_flag(engine->state, GameState::ROOMMENU | GameState::INGAME | GameState::FINISHED)) {
            GameState target = GameState::PLAYMENU;
            engine->request_change_state(target);
            return;
        }
#endif
        // TODO: the room_running will be fixed in the future with new msg.
        if (gd->room && gd->room->state == ROOM_RUNNING) {
            GameState target = GameState::INGAME;
            engine->request_change_state(target);
            return;
        }
    };
    engine->om.add_object(sc, (*z)++);

    int text_size = 32;
    Color text_color = RED;
    Text *t = cText(engine, state, std::string(), text_size, text_color, {0,0});
    t->btext = gd->text_buffer;
    t->rec.y = wsize.y - text_size;
    t->show = false;
    engine->om.add_object(t, (*z)++);

    // 5 Seconds Timer to show the text buffer if not displayed yet
    std::chrono::milliseconds ms = std::chrono::milliseconds(5000);
    Timer *ttimer = new Timer(ms);
    ttimer->tt = LOOP;
    ttimer->state = state;

    // Define Callbacks
    ttimer->miss_callback = [t, gd, ttimer]() {
#ifndef __EMSCRIPTEN__
        // Declare lock guard mutex
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        // If has not been displayed yet, display it (new message)
        if (!gd->text_buffer_displayed) {
            gd->text_buffer_displayed = true;
            t->show = true;
            ttimer->start_timer();
        }
    };

    // Clean up callback after every 5 seconds
    ttimer->callback = [t, gd, ttimer]() {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        t->show = false; // Just hide it
    };

    // Show the timer
    engine->om.add_object(ttimer, (*z)++);
    ttimer->start_timer();
}

static void gameInit(ArsEng *engine) {
    GameData *gd = new GameData();
    gd->player1.health = 4;
    gd->player2.health = 4;
    gd->round_needle_count = 5;
    gd->pstate = PlayerState::PLAYER1;
    gd->client = new Client();
    gd->client->callback = client_handler;
    gd->player = {};
    gd->room = nullptr;
    gd->text_buffer = new std::string();
    gd->text_buffer_displayed = false;
#ifndef __EMSCRIPTEN__
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
    // Why is this TODO ? Specify reason please hehe
    //TODO(1):
    (void)canvas_size;

    KeyHandler *kh = new KeyHandler();
    kh->engine_state = &engine->state;
    int kh_id = engine->om.add_object(kh, z++);

    // Load Object
    initALLObject  (engine, kh_id, &z);
    initTestObject (engine, kh_id, &z);
    initMenu       (engine, kh_id, &z);
    initSettings   (engine, kh_id, &z);
    initPlayMenu   (engine, kh_id, &z);
    initInGame     (engine, kh_id, &z);
    initRoomMenu   (engine, kh_id, &z);
}

static void gameDeinit(ArsEng *engine) {
    GameData *gd = (GameData *)engine->additional_data;
    if (gd) {
        // I'll keep the old version just in case wee don't need the delay
        // if (gd->client) { gd->client->done = true; }
        // Clean up ? Do we Need it ?
        if (gd->client) {
            gd->client->done = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
#ifndef __EMSCRIPTEN__
        if (gd->_net.joinable()) { gd->_net.join(); }
#endif
        delete gd->client;
        delete gd->text_buffer;
        delete gd;
    }
}

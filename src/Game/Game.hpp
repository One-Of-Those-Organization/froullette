#pragma once
// NOTE: Future work or rewrite please use `clay` layouting lib to make it
// easier

// TODO: make the needle pos shared between all the client (if all done)

#include "../Message/Message.hpp"
#include "../Object/Balls.hpp"
#include "../Object/Button.hpp"
#include "../Object/Cursor.hpp"
#include "../Object/Desk.hpp"
#include "../Object/Hbox.hpp"
#include "../Object/KeyHandler.hpp"
#include "../Object/Needle.hpp"
#include "../Object/NeedleContainer.hpp"
#include "../Object/Script.hpp"
#include "../Object/Text.hpp"
#include "../Object/TextInput.hpp"
#include "../Object/Timer.hpp"
#include "../Shared/LobbyStatus.hpp"
#include "../Shared/Player.hpp"
#include "../Shared/Room.hpp"
#include "ArsEng.hpp"
#include "Client.hpp"
#include "GameState.hpp"
#include <ctime>
#include <queue>
#include <thread>

struct GameData {
#ifndef __EMSCRIPTEN__
  std::mutex mutex;
#endif
  size_t round_needle_count;
  Client *client;
#ifndef __EMSCRIPTEN__
  std::thread _net;
#endif
  Room *room;

  Player player;
  Player oplayer;
  int round_counter = 1;

  std::string url_buffer;
  std::string buffer;

  std::string *text_buffer;
  bool text_buffer_displayed;

  std::queue<int> dragged_obj_qq; // queue for dragged_obj
  LobbyStatus ls;
  GameAction action;
  bool lock_action;
  NeedleContainer *needle_container;
  int winner_id;
};

static void client_handler(mg_connection *c, int ev, void *ev_data) {
  GameData *gd = (GameData *)c->fn_data;
  if (!gd) {
    TraceLog(LOG_INFO, "NET: Failed to get the gamedata on the network thread");
    return;
  }
  Client *client = gd->client;
  switch (ev) {
  case MG_EV_WS_OPEN: {
    TraceLog(LOG_INFO, "NET: WebSocket handshake complete!");
    if (!client)
      return;
    client->on_connected();
  } break;
  case MG_EV_WS_MSG: {
    mg_ws_message *wm = (mg_ws_message *)ev_data;
    if ((wm->flags & 0x0f) != WEBSOCKET_OP_BINARY)
      break;
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
      case LOBBY_STATUS: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        gd->ls.count = pd.data.LobbyStatus_obj.count;
        memcpy(gd->ls.ready, pd.data.LobbyStatus_obj.ready,
               2); // ready index 0 is the current client.
      } break;

      case READY_STATUS: {
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

      case PLAYER_INFO: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        char name[3] = "EN";
        Player *target = &gd->oplayer;
        if (pd.data.Player_obj->id == gd->player.id) {
          target = &gd->player;
          strcpy(name, "ME");
        }
        TraceLog(LOG_INFO,
                 "Get %s player info with the content: id(%d), hp(%d)", name,
                 pd.data.Player_obj->id, pd.data.Player_obj->health);
        *target = *pd.data.Player_obj;
        delete pd.data.Player_obj;
      } break;

      case EXIT_ROOM: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        delete gd->room;
        gd->room = nullptr;
      } break;
      case GAME_TURN_UPDATE: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        PlayerState state = (PlayerState)pd.data.Int;
        if (state == gd->player.turn) {
          gd->lock_action = false;
        } else {
          gd->lock_action = true;
        }
        gd->room->turn = state;
        TraceLog(LOG_INFO, "NET: Turn Update received: %d", (int)state);
      } break;

      case GAME_END: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        gd->winner_id = pd.data.Int;
        gd->room->state = ROOM_FINISHED;
      } break;

      case GAME_FINISHED_PREMATURELY: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        if (gd->room)
          gd->room->state = ROOM_ACTIVE;
        gd->oplayer = {};
      } break;

      case GAME_START: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        if (gd->room) {
          gd->room->state = ROOM_RUNNING;
          if (gd->room->turn == PlayerState::PLAYER1) {
            gd->player.turn = PlayerState::PLAYER1;
          } else {
            gd->player.turn = PlayerState::PLAYER2;
          }
        }
      } break;
      case GAME_NEEDLE_DATA: {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(gd->mutex);
#endif
        if (gd->needle_container) {
          MinimalNeedle *needles_info = (MinimalNeedle *)pd.data.Byte.data;
          size_t count = pd.data.Byte.len / sizeof(MinimalNeedle);
          for (size_t i = 0; i < count; ++i) {
            for (Needle *n : gd->needle_container->needles) {
              if (n->shared_id == needles_info[i].id) {
                n->used = (uint8_t)needles_info[i].used;
                // TODO: reset the pos. maybe only active player can move around
                // and synced.
                break;
              }
            }
          }
        }
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
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    const char *fm = TextFormat("NET: Error: %s", (char *)ev_data);
    TraceLog(LOG_ERROR, "%s", fm);
    gd->text_buffer_displayed = false;
    *gd->text_buffer = fm;
  } break;
  case MG_EV_CLOSE: {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    TraceLog(LOG_INFO, "NET: Connection closed");
    if (gd->room)
      delete gd->room;
    gd->room = nullptr;
    gd->player = Player{};
    gd->player.health = MAX_PLAYER_HEALTH;
    if (client) {
      client->on_disconnected();
    }
  } break;
  default:
    break;
  }
}

static bool start_connection(ArsEng *engine) {
  GameData *gd = (GameData *)engine->additional_data;
#ifndef __EMSCRIPTEN__
  std::lock_guard<std::mutex> lock(gd->mutex);
#endif
  if (gd->url_buffer.empty())
    return false;
  gd->client->url = gd->url_buffer.c_str();
  if (!gd->client->connect((void *)gd)) {
    const char *fm = "NET: Failed to connect to the specified server";
    TraceLog(LOG_INFO, "%s", fm);
    gd->text_buffer_displayed = false;
    *gd->text_buffer = fm;
    return false;
  }
  Message msg = {};
  msg.type = MessageType::GIVE_ID;
  msg.response = MessageType::NONE;
  gd->client->send(msg);
  return true;
}

static TextInput *cTextInput(ArsEng *engine, const char *placeholder,
                             std::string *buffer, int text_size, int padding,
                             GameState state, Vector2 pos) {
  TextInput *ti = new TextInput(placeholder);
  ti->font = &engine->font;
  ti->font_size = text_size;
  ti->state = state;
  ti->rec = {pos.x, pos.y, 100, 100};
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
                       std::function<void()> callback) {
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

static Text *cText(ArsEng *engine, GameState state, std::string text,
                   size_t text_size, Color color, Vector2 pos) {
  auto text1 = new Text();
  text1->text = text;
  text1->font = &engine->font;
  text1->text_size = text_size;
  text1->text_color = color;
  text1->rec = {pos.x, pos.y, 100, 100};
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
  Vector2 wsize = {(float)engine->canvas.texture.width,
                   (float)engine->canvas.texture.height};
  GameState state = GameState::INGAME;
  KeyHandler *kh = (KeyHandler *)engine->om.get_object(kh_id);
  GameData *gd = (GameData *)engine->additional_data;

  if (!kh)
    TraceLog(LOG_INFO, "Failed to register keybinding to the ingame state");
  else {
    kh->add_new(KEY_Q, state, [engine, gd]() {
      Message msg = {};
      msg.type = EXIT_ROOM;
      msg.response = NONE;
      gd->client->send(msg);
      if (gd->room) {
        delete gd->room;
        gd->room = nullptr; // NOTE: IDK if this is the best approach
        // but yeah...
      }
      engine->request_change_state(GameState::ROOMMENU);
    });
  }

  int text_size = 32;
  int padding = 20;
  Texture2D *exit_icon = engine->tm.get_texture("exit");
  Button *btnexit =
      cButton(engine, "", text_size, padding, state, {0, 0}, [engine, gd]() {
        Message msg = {};
        msg.type = EXIT_ROOM;
        msg.response = NONE;
        gd->client->send(msg);
        if (gd->room) {
          delete gd->room;
          gd->room = nullptr; // NOTE: IDK if this is the best approach
                              // but yeah...
        }
        engine->revert_state();
      });
  btnexit->text = exit_icon;
  btnexit->calculate_rec();
  btnexit->rec.x = padding;
  btnexit->rec.y = padding;
  btnexit->rec.width = 64;
  btnexit->rec.height = 64;
  engine->om.add_object(btnexit, (*z)++);

  Texture2D *player2_text =
      engine->tm.load_texture("p2", "./assets/DoctorFix1024.png");
  auto p2 = new Object();
  p2->rec =
      Rectangle{0, 0, player2_text->width / 8.0f, player2_text->height / 8.0f};
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
  gd->needle_container = ns;
  engine->om.add_object(ns, (*z)++);

  std::chrono::milliseconds ms = std::chrono::milliseconds(200);
  Timer *drag_timer = new Timer(ms);
  drag_timer->tt = LOOP;
  drag_timer->state = state;
  drag_timer->callback = [engine, gd]() {
    if (gd->dragged_obj_qq.empty())
      return;
    int id = gd->dragged_obj_qq.front();
    gd->dragged_obj_qq.pop();
    if (engine->dragged_obj == id)
      return;
    engine->dragged_obj = id;
  };
  engine->om.add_object(drag_timer, (*z)++);
  drag_timer->start_timer();

  Texture2D *needle_text =
      engine->tm.load_texture("needle", "./assets/needle_normal.png");
  const Rectangle needle_pos = {
      .x = desk->rec.x,
      .y = desk->rec.y + (desk->rec.y / 6.0f),
      .width = desk->rec.width,
      .height = desk->rec.height - (desk->rec.height / 6.0f),
  };

  ns->rec = needle_pos;
  ns->color = GetColor(0xf0f0f055);
  ns->state = state;

  srand(time(0));
  for (size_t i = 0; i < gd->round_needle_count; i++) {
    auto needle = new Needle();
    const int padding = 10;

    Rectangle current_pos = {
        .x = padding + needle_pos.x + (i * 6),
        .y = needle_pos.y,
        .width = 10,
        .height = 20,
    };

    needle->shared_id = i;
    needle->max_rec = needle_pos;
    needle->text = needle_text;
    needle->engine_dragging = &engine->dragging;
    needle->engine_dragged_id = &engine->dragged_obj;
    needle->dragged_qq = &gd->dragged_obj_qq;
    needle->rec = current_pos;
    needle->curpos = &engine->canvas_cursor;
    needle->type = NeedleType::NT_BLANK;
    needle->state = state;
    needle->used = true; // NOTE: default set to true so it only rendered when
                         // the server give it a go.
    needle->callback = [gd](Needle *n) {
#ifndef __EMSCRIPTEN__
      std::lock_guard<std::mutex> lock(gd->mutex);
#endif
      if (gd->room->turn != gd->player.turn)
        return;
      if (gd->lock_action)
        return;

      gd->action.type = GameActionType::INJECT;
      gd->action.data.i32 = n->shared_id;
      gd->lock_action = true;
      Message msg = {};
      msg.type = GAME_PLAYER_UPDATE;
      msg.response = NONE;
      msg.data.action = &gd->action;
      gd->client->send(msg);
    };
    engine->om.add_object(needle, (*z)++);
    ns->needles.push_back(needle);
  }
  Vector2 bigcanvas_size = {(float)engine->bigcanvas.texture.width,
                            (float)engine->bigcanvas.texture.height};

  Color text_color = WHITE;
  text_size = 64;
  Text *tturn =
      cText(engine, state, "Your Turn", text_size, text_color, {0, 0});
  Vector2 tturn_len = tturn->calculate_len();
  tturn->rec.x = (bigcanvas_size.x - tturn_len.x) / 2.0f;
  tturn->rec.y = (bigcanvas_size.y - (padding + tturn_len.y));
  tturn->show = false;
  engine->om.add_object(tturn, (*z)++);

  size_t icon_size = 64;
  padding = 1;
  HBox *hbox = new HBox();
  hbox->state = state;
  hbox->rec.width = (icon_size * MAX_PLAYER_HEALTH);
  hbox->rec.height = icon_size + padding;
  hbox->rec.x = (bigcanvas_size.x - hbox->rec.width) / 2.0f;
  hbox->rec.y = tturn->rec.y - (icon_size + padding);
  hbox->padding = padding;

  hbox->al = Alignment::CENTER;
  hbox->draw_in_canvas = false;
  engine->om.add_object(hbox, (*z)++);

  Texture *braintxt = engine->tm.load_texture("brain", "./assets/brain.png");
  if (!braintxt) {
    TraceLog(LOG_INFO, "Failed to load the brain texture somethis is wrong.");
    return;
  };
  Object *all_brain[5] = {};

  for (int i = 0; i < MAX_PLAYER_HEALTH; i++) {
    Object *obrain = new Object();
    obrain->text = braintxt;
    obrain->rec = {0, 0, (float)icon_size, (float)icon_size};
    obrain->state = state;
    obrain->show = true;
    obrain->color = BLACK;
    obrain->draw_in_canvas = false;
    engine->om.add_object(obrain, (*z)++);
    hbox->add_child(obrain);
    hbox->position_child();
    all_brain[i] = obrain;
  }
  hbox->position_child();

  Script *ingame_sc = new Script();
  ingame_sc->state = state;
  ingame_sc->callback = [gd, tturn, all_brain, hbox]() {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    if (gd->room && gd->room->turn == gd->player.turn) {
      tturn->show = true;
    } else {
      tturn->show = false;
    }
    int i = 0;
    for (Object *brain : all_brain) {
      if (i++ < gd->player.health)
        brain->color = WHITE;
      else
        brain->color = BLACK;
    }
  };
  engine->om.add_object(ingame_sc, (*z)++);
}

static void initMenu(ArsEng *engine, int kh_id, int *z) {
  Vector2 wsize = {(float)engine->bigcanvas.texture.width,
                   (float)engine->bigcanvas.texture.height};

#ifdef DEBUG_
  KeyHandler *kh = (KeyHandler *)engine->om.get_object(kh_id);
  if (kh) {
    GameData *gd = (GameData *)engine->additional_data;
    kh->add_new(KEY_W, GameState::MENU, [engine, gd]() {
      engine->request_change_state(GameState::FINISHED);
    });
  }
#endif
  (void)kh_id;
  GameState state = GameState::MENU;
  size_t title_size = 64;
  Color title_color = WHITE;

  Text *title1 = cText(engine, state, "Fate", title_size, title_color, {0, 0});
  Vector2 title1_len = title1->calculate_len();
  title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
  title1->rec.y = (wsize.y / 2.0f) - title1_len.y;
  engine->om.add_object(title1, (*z)++);

  Text *title2 =
      cText(engine, state, "Roullete", title_size, title_color, {0, 0});
  Vector2 title2_len = title2->calculate_len();
  title2->rec.x = (wsize.x - title2_len.x) / 2.0f;
  title2->rec.y = (wsize.y / 2.0f);
  engine->om.add_object(title2, (*z)++);

  size_t text_size = 36;
  size_t padding = 20;

  Button *btn1 =
      cButton(engine, "Start", text_size, padding, state, {0, 0}, [engine]() {
        engine->request_change_state(GameState::PLAYMENU);
      });
  btn1->calculate_rec();
  btn1->rec.x = (wsize.x - btn1->rec.width) / 2.0f;
  btn1->rec.y = wsize.y - (btn1->rec.height + padding * 5);
  engine->om.add_object(btn1, (*z)++);

  Texture2D *settings_cog =
      engine->tm.load_texture("cogs", "./assets/settings.png");
  Button *btn2 =
      cButton(engine, "", text_size, padding, state, {0, 0}, [engine]() {
        engine->request_change_state(GameState::SETTINGS);
      });
  btn2->text = settings_cog;
  btn2->calculate_rec();
  btn2->rec.x = btn1->rec.x + btn1->rec.width + padding;
  btn2->rec.y = btn1->rec.y;
  btn2->rec.width = btn1->rec.height;
  btn2->rec.height = btn1->rec.height;
  engine->om.add_object(btn2, (*z)++);

  Texture2D *exit_icon = engine->tm.load_texture("exit", "./assets/exit.png");
#ifndef __EMSCRIPTEN__
  Button *btn3 = cButton(engine, "", text_size, padding, state, {0, 0},
                         [engine]() { engine->req_close = true; });
  btn3->text = exit_icon;
  btn3->calculate_rec();
  btn3->rec.x = btn1->rec.x - (btn1->rec.height + padding);
  btn3->rec.y = btn1->rec.y;
  btn3->rec.width = btn1->rec.height;
  btn3->rec.height = btn1->rec.height;
  engine->om.add_object(btn3, (*z)++);
#else
  (void)exit_icon;
#endif
}

static void initPlayMenu(ArsEng *engine, int kh_id, int *z) {
  Vector2 wsize = {(float)engine->bigcanvas.texture.width,
                   (float)engine->bigcanvas.texture.height};
  GameState state = GameState::PLAYMENU;
  int padding = 20;
  int text_size = 32;
  int title_size = 64;
  Color title_color = WHITE;

  KeyHandler *kh = (KeyHandler *)engine->om.get_object(kh_id);
  if (!kh)
    TraceLog(LOG_INFO, "Failed to register keybinding to the playmenu state");
  else {
    kh->add_new(KEY_Q, state, [engine]() {
      if (engine->active < 0)
        engine->request_change_state(GameState::MENU);
    });
  }
  GameData *gd = (GameData *)engine->additional_data;

  Texture2D *exit_icon = engine->tm.get_texture("exit");
  if (!exit_icon) {
    TraceLog(LOG_FATAL, "Failed to get the EXIT TEXTURE!");
    return;
  }

  Text *title1 =
      cText(engine, state, "Get ready", title_size, title_color, {0, 0});
  Vector2 title1_len = title1->calculate_len();
  title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
  title1->rec.y = title1_len.y + padding;
  engine->om.add_object(title1, (*z)++);

  gd->url_buffer = "127.0.0.1:8000";
  TextInput *url =
      cTextInput(engine, "Enter ip:port", &gd->url_buffer, text_size, padding,
                 state, {wsize.x / 2.0f, title1->rec.y + title1->rec.height});
  url->rec.width = (wsize.x - padding * 5) / 2.0f;
  url->rec.x = (wsize.x - url->rec.width) / 2.0f;
  engine->om.add_object(url, (*z)++);

  TextInput *id = cTextInput(
      engine, "Room id (insert only on connect)", &gd->buffer, text_size,
      padding, state,
      {wsize.x / 2.0f, url->rec.y + url->rec.height + (padding * 2)});
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

  Button *btncreate = cButton(engine, "Create room", text_size, padding, state,
                              {0, 0}, [engine, gd]() {
                                if (!gd->client->c) {
                                  if (!start_connection(engine))
                                    return;
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

  Button *btnconnect = cButton(engine, "Connect to room", text_size, padding,
                               state, {0, 0}, [engine, gd]() {
                                 if (!gd->client->c) {
                                   if (!start_connection(engine))
                                     return;
                                 }
                                 Message msg = {};
                                 msg.type = CONNECT_ROOM;
                                 msg.response = NONE;
                                 strncpy(msg.data.String, gd->buffer.data(),
                                         MAX_MESSAGE_STRING_SIZE);
                                 gd->client->send(msg);
                               });
  btnconnect->calculate_rec();
  engine->om.add_object(btnconnect, (*z)++);
  hbox->add_child(btnconnect);
  hbox->position_child();

  Button *btn1 = cButton(engine, "", 0, padding, state, {0, 0}, [engine]() {
    engine->request_change_state(GameState::MENU);
  });
  btn1->text = exit_icon;
  btn1->rec.width = 64;
  btn1->rec.height = 64;
  btn1->rec.x = padding;
  btn1->rec.y = padding;
  engine->om.add_object(btn1, (*z)++);
}

static void initSettings(ArsEng *engine, int kh_id, int *z) {
  Vector2 wsize = {(float)engine->bigcanvas.texture.width,
                   (float)engine->bigcanvas.texture.height};
  GameState state = GameState::SETTINGS;
  size_t title_size = 64;

#ifndef __EMSCRIPTEN__
  size_t text_size = 32;
#endif
  size_t padding = 20;
  Color title_color = WHITE;

  KeyHandler *kh = (KeyHandler *)engine->om.get_object(kh_id);
  if (!kh)
    TraceLog(LOG_INFO, "Failed to register keybinding to the settings state");
  else {
    kh->add_new(KEY_Q, state, [engine]() { engine->revert_state(); });
  }

  Text *title1 =
      cText(engine, state, "Settings", title_size, title_color, {0, 0});
  Vector2 title1_len = title1->calculate_len();
  title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
  title1->rec.y = title1_len.y + padding;
  engine->om.add_object(title1, (*z)++);

#ifndef __EMSCRIPTEN__
  Text *restext =
      cText(engine, state, "Resolution", text_size, title_color, {0, 0});
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

  Button *btnfull =
      cButton(engine, "Toggle Fullscreen", text_size, padding, state, {0, 0},
              [engine]() { engine->request_fullscreen(); });
  btnfull->calculate_rec();
  engine->om.add_object(btnfull, (*z)++);
  hbox->add_child(btnfull);
  hbox->position_child();

  Button *btnhd = cButton(engine, "720p", text_size, padding, state, {0, 0},
                          [engine]() { engine->request_resize({1280, 720}); });
  btnhd->calculate_rec();
  engine->om.add_object(btnhd, (*z)++);
  hbox->add_child(btnhd);
  hbox->position_child();

  Button *btnfhd =
      cButton(engine, "1080p", text_size, padding, state, {0, 0},
              [engine]() { engine->request_resize({1920, 1080}); });
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
  Button *btn1 = cButton(engine, "", 0, padding, state, {0, 0},
                         [engine]() { engine->revert_state(); });
  btn1->text = exit_icon;
  btn1->rec.width = 64;
  btn1->rec.height = 64;
  btn1->rec.x = padding;
  btn1->rec.y = padding;
  engine->om.add_object(btn1, (*z)++);
}

static void initRoomMenu(ArsEng *engine, int kh_id, int *z) {
  Vector2 wsize = {(float)engine->bigcanvas.texture.width,
                   (float)engine->bigcanvas.texture.height};
  GameData *gd = (GameData *)engine->additional_data;
  GameState state = GameState::ROOMMENU;
  KeyHandler *kh = (KeyHandler *)engine->om.get_object(kh_id);
  if (!kh)
    TraceLog(LOG_INFO, "Failed to register keybinding to the ingame state");
  else {
    kh->add_new(KEY_Q, state, [engine, gd]() {
      Message msg = {};
      msg.type = EXIT_ROOM;
      msg.response = NONE;
      gd->client->send(msg);
      if (gd->room) {
        delete gd->room;
        gd->room = nullptr; // NOTE: IDK if this is the best approach
                            // but yeah...
      }
      engine->request_change_state(GameState::PLAYMENU);
    });
  }

  size_t text_size = 32;
  size_t title_size = 48;
  size_t padding = 20;
  Color title_color = WHITE;

  Text *text_id =
      cText(engine, state, "Room id: _", title_size, title_color, {0, 0});
  Vector2 text_id_len = text_id->calculate_len();
  text_id->rec.x = (wsize.x - text_id_len.x) / 2.0f;
  text_id->rec.y = (wsize.y / 4.0f) - text_id_len.y;
  engine->om.add_object(text_id, (*z)++);

  Texture2D *exit_icon = engine->tm.get_texture("exit");
  if (!exit_icon) {
    TraceLog(LOG_FATAL, "Failed to get the EXIT TEXTURE!");
    return;
  }

  Button *btn1 = cButton(engine, "", 0, padding, state, {0, 0}, [engine]() {
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

  Button *btn2 =
      cButton(engine, "Ready", text_size, padding, state, {0, 0}, [engine]() {
        GameData *gd = (GameData *)engine->additional_data;
        Message msg = {};
        msg.type = TOGGLE_READY;
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
      std::string fmt = std::format("Room id: {}", gd->room->id);
      text_id->text = fmt;
      text_id->rec.x = (wsize.x - text_id->calculate_len().x) / 2.0;
    } else if (has_flag(engine->state, GameState::ROOMMENU)) {
      engine->request_change_state(GameState::PLAYMENU);
    }

    // NOTE: This will be bad for performance but i guess for simplicity and
    // for the result of my bad design
    //       legit this is so bad...
    if (gd->player.ready && btn2->str != "Ready") {
      btn2->str = "Ready";
      btn2->calculate_rec();
      btn2->rec.x = (wsize.x - btn2->rec.width) / 2.0f;
    } else if (!gd->player.ready && btn2->str != "Unready") {
      btn2->str = "Unready";
      btn2->calculate_rec();
      btn2->rec.x = (wsize.x - btn2->rec.width) / 2.0f;
    }
  };

  engine->om.add_object(sc, (*z)++);

  Color text_color = WHITE;
  Text *pcounttxt =
      cText(engine, state, "Player(0/2)", text_size, text_color, {0, 0});
  ;
  Vector2 pcounttxt_len = pcounttxt->calculate_len();
  pcounttxt->rec.x = wsize.x - pcounttxt_len.x;
  pcounttxt->rec.y = wsize.y - pcounttxt_len.y;
  engine->om.add_object(pcounttxt, (*z)++);

  std::chrono::milliseconds ms = std::chrono::milliseconds(100);
  Timer *tu_timer = new Timer(ms);
  tu_timer->tt = LOOP;
  tu_timer->state = state;
  tu_timer->callback = [pcounttxt, gd, wsize]() {
    const char *count_update = TextFormat("Player(%d/2)", gd->ls.count);
    if (strcmp(count_update, pcounttxt->text.c_str()) != 0) {
      pcounttxt->text = count_update;
      pcounttxt->rec.x = wsize.x - pcounttxt->calculate_len().x;
    }
  };
  engine->om.add_object(tu_timer, (*z)++);
  tu_timer->start_timer();
}

// TODO: working on this
static void initFinishMenu(ArsEng *engine, int kh_id, int *z) {
  Vector2 wsize = {(float)engine->bigcanvas.texture.width,
                   (float)engine->bigcanvas.texture.height};
  KeyHandler *kh = (KeyHandler *)engine->om.get_object(kh_id);
  GameState state = GameState::FINISHED;
  GameData *gd = (GameData *)engine->additional_data;

  if (!kh)
    TraceLog(LOG_INFO, "Failed to register keybinding to the ingame state");
  else {
    kh->add_new(KEY_Q, state, [engine, gd]() {
      engine->request_change_state(GameState::ROOMMENU);
#ifndef __EMSCRIPTEN__
      std::lock_guard<std::mutex> lock(gd->mutex);
#endif
      gd->winner_id = -1;
    });
  }

  size_t title_size = 64;
  Color title_color = WHITE;

  Text *title1 =
      cText(engine, state, "Game Finished", title_size, title_color, {0, 0});
  Vector2 title1_len = title1->calculate_len();
  title1->rec.x = (wsize.x - title1_len.x) / 2.0f;
  title1->rec.y = (wsize.y / 2.0f) - title1_len.y;
  engine->om.add_object(title1, (*z)++);

  Text *title2 =
      cText(engine, state, "YOU SOME", title_size, title_color, {0, 0});
  Vector2 title2_len = title2->calculate_len();
  title2->rec.x = (wsize.x - title2_len.x) / 2.0f;
  title2->rec.y = (wsize.y / 2.0f);
  engine->om.add_object(title2, (*z)++);

  Script *sc = new Script();
  sc->state = state;
  sc->callback = [title2, gd]() {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    char buff[5] = " WIN";
    if (gd->winner_id < 0)
      return;
    if ((uint32_t)gd->winner_id != gd->player.id)
      strcpy(buff, "LOSE");
    const char *stuff = TextFormat("YOU %s", buff);
    title2->text = stuff;
  };
  engine->om.add_object(sc, (*z)++);

  int text_size = 32;
  int padding = 20;
  Button *btn1 = cButton(engine, "Continue", text_size, padding, state, {0, 0},
                         [engine, gd]() {
                           engine->request_change_state(GameState::ROOMMENU);
#ifndef __EMSCRIPTEN__
                           std::lock_guard<std::mutex> lock(gd->mutex);
#endif
                           gd->winner_id = -1;
                         });
  btn1->calculate_rec();
  btn1->rec.x = (wsize.x - btn1->rec.width) / 2.0f;
  btn1->rec.y = wsize.y - (btn1->rec.height + padding * 5);
  engine->om.add_object(btn1, (*z)++);
}

static void initALLObject(ArsEng *engine, int kh_id, int *z) {
  Vector2 wsize = {(float)engine->bigcanvas.texture.width,
                   (float)engine->bigcanvas.texture.height};
  (void)kh_id;
  GameState state = GameState::ALL;
  Script *sc = new Script();
  sc->state = state;
  GameData *gd = (GameData *)engine->additional_data;
  sc->callback = [engine, gd]() {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    if (gd->room &&
        !has_flag(engine->state, GameState::ROOMMENU | GameState::INGAME |
                                     GameState::FINISHED)) {
      GameState target = GameState::ROOMMENU;
      if (gd->room->state == ROOM_RUNNING)
        target = GameState::INGAME;
      engine->request_change_state(target);
      return;
    }

#ifndef DEBUG_
    if (!gd->room && gd->player.id == 0 &&
        has_flag(engine->state, GameState::ROOMMENU | GameState::INGAME |
                                    GameState::FINISHED)) {
      GameState target = GameState::PLAYMENU;
      engine->request_change_state(target);
      return;
    }
    if (gd->room && gd->room->state == ROOM_ACTIVE &&
        engine->state == GameState::INGAME) {
      GameState target = GameState::ROOMMENU;
      engine->request_change_state(target);
      return;
    }
    if (gd->room && gd->room->state == ROOM_RUNNING) {
      GameState target = GameState::INGAME;
      engine->request_change_state(target);
      return;
    }
#endif
    if (gd->room && gd->room->state == ROOM_FINISHED) {
      gd->room->state = ROOM_ACTIVE;
      engine->request_change_state(GameState::FINISHED);
    }
  };
  engine->om.add_object(sc, (*z)++);

  int text_size = 32;
  Color text_color = RED;
  Text *t = cText(engine, state, std::string(), text_size, text_color, {0, 0});
  t->btext = gd->text_buffer;
  t->rec.y = wsize.y - text_size;
  t->show = false;
  engine->om.add_object(t, (*z)++);

  std::chrono::milliseconds ms = std::chrono::milliseconds(5000);
  Timer *ttimer = new Timer(ms);
  ttimer->tt = LOOP;
  ttimer->state = state;
  ttimer->miss_callback = [t, gd, ttimer]() {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(gd->mutex);
#endif
    if (!gd->text_buffer_displayed) {
      gd->text_buffer_displayed = true;
      t->show = true;
      ttimer->start_timer();
    }
  };
  ttimer->callback = [t]() { t->show = false; };
  engine->om.add_object(ttimer, (*z)++);
  ttimer->start_timer();

#ifndef MOBILE
  Texture2D *cursor_text =
      engine->tm.load_texture("cursor", "./assets/cursor.png");
  Cursor *cr = new Cursor();
  cr->rec = {};
  cr->state = state;
  cr->text = cursor_text;
  cr->cursor = &engine->bigcanvas_cursor;
  cr->draw_in_canvas = false;
  engine->om.add_object(cr, 9999);
#endif
}

static void gameInit(ArsEng *engine) {
  GameData *gd = new GameData();
  gd->round_needle_count = 5;
  gd->client = new Client();
  gd->client->callback = client_handler;
  gd->player = {};
  gd->oplayer = {};
  gd->room = nullptr;
  gd->ls = {1, {false, false}};
  gd->text_buffer = new std::string();
  gd->text_buffer_displayed = false;
  gd->needle_container = nullptr;
  gd->action = {};
  gd->lock_action = false;
  gd->winner_id = -1;
#ifndef __EMSCRIPTEN__
  gd->_net = std::thread([gd]() {
    mg_log_level = MG_LL_ERROR;
    if (gd && gd->client) {
      gd->client->loop(100);
    }
  });
#endif

  engine->additional_data = (void *)gd;
  int z = 1;

  KeyHandler *kh = new KeyHandler();
  kh->engine_state = &engine->state;
  int kh_id = engine->om.add_object(kh, z++);

  // Load Object
  initALLObject(engine, kh_id, &z);
  initTestObject(engine, kh_id, &z);
  initMenu(engine, kh_id, &z);
  initSettings(engine, kh_id, &z);
  initPlayMenu(engine, kh_id, &z);
  initInGame(engine, kh_id, &z);
  initRoomMenu(engine, kh_id, &z);
  initFinishMenu(engine, kh_id, &z);
}

static void gameDeinit(ArsEng *engine) {
  GameData *gd = (GameData *)engine->additional_data;
  if (gd) {
    if (gd->client) {
      gd->client->done = true;
    }
#ifndef __EMSCRIPTEN__
    if (gd->_net.joinable()) {
      gd->_net.join();
    } // NOTE: on thread model no need no delay because `join` will wait
      // that thread to exit.
#endif
    delete gd->client;
    delete gd->text_buffer;
    delete gd;
  }
}

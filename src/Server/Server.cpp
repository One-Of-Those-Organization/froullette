#define _SERVER

#include "Server.hpp"
#include "../Message/Message.hpp"
#include "../Shared/Helper.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#define brp asm("int3")

static std::unordered_map<mg_connection *, uint32_t> player_conmap = {};
static std::vector<Room *> created_room = {};

static inline size_t ws_send(struct mg_connection *c, Message *msg) {
  uint8_t out[MAX_MESSAGE_BIN_SIZE];
  size_t n = generate_network_field(msg, out);
  printf("Generated data with size: %zu\n", n);
  return mg_ws_send(c, out, n, WEBSOCKET_OP_BINARY);
}

Room *find_free_room(Server *server) {
  for (size_t i = 0; i < MAX_ROOM_COUNT; i++) {
    if (server->rooms[i].state == ROOM_FREE) {
      return &server->rooms[i];
    }
  }
  return nullptr;
}

static std::vector<MinimalNeedle>
initialize_needle(size_t needle_count, size_t live_needles_count, Room *r) {
  std::vector<uint8_t> needle_types;
  for (size_t i = 0; i < live_needles_count; ++i)
    needle_types.push_back(1);
  for (size_t i = 0; i < needle_count - live_needles_count; ++i)
    needle_types.push_back(0);

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::shuffle(needle_types.begin(), needle_types.end(),
               std::default_random_engine(seed));

  std::vector<MinimalNeedle> mn;
  mn.reserve(needle_count);
  r->needles.clear();

  // NOTE: plaese sync the id with the client right now it is since
  // the 2 of them use 0..4
  for (size_t i = 0; i < needle_count; ++i) {
    _MinimalNeedle needle = {(uint8_t)i, false, needle_types[i], false};
    r->needles.push_back(needle);
    mn.push_back({needle.id, needle.used});
  }
  return mn;
}

static void timer_fn(void *arg) {
  (void)arg;
  for (auto &r : created_room) {
    Player *p = r->players[0];
    Player *op = r->players[1];
    if (r->state == ROOM_ACTIVE) {
      Message msg = {};
      msg.type = LOBBY_STATUS;
      msg.response = NONE;
      bool pready = (p)   ? p->ready  : false;
      bool opready = (op) ? op->ready : false;
      msg.data.LobbyStatus_obj = {r->player_len, {pready, opready}};
      if (p && p->con)
        ws_send(p->con, &msg);
      if (op && op->con)
        ws_send(op->con, &msg);
    }
  }
}

static void ws_handler(mg_connection *c, int ev, void *ev_data) {
  Server *server = (Server *)c->fn_data;
  switch (ev) {
  case MG_EV_HTTP_MSG: {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
      mg_ws_upgrade(c, hm, NULL);
      c->data[0] = 'W';
    }
    break;
  }
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

      Message reply{};
      switch (pd.type) {
      case LOGIN_ID: {
        if (server->players.count(pd.data.Int) > 0) {
          server->players[pd.data.Int].con = c;
          reply.type = OK;
          reply.response = LOGIN_ID;
          break;
        }
      } break;
      case GIVE_ID: {
        reply.type = HERE_ID;
        reply.data.Int = server->ccount;
        server->players[server->ccount] = Player{
            .id = server->ccount,
            .health = MAX_PLAYER_HEALTH,
            .con = c,
            .pe = { .type = 3, .data = 0, .used = false },
            .ready = false,
            .turn = PlayerState::PLAYER1, // NOTE: update this on room
                                          // enter.
            .items = {},
        };
        player_conmap[c] = server->ccount;
        ++server->ccount;
      } break;
      case CREATE_ROOM: {
        if (player_conmap.count(c) <= 0) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::CREATE_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Please do GIVE_ID first to register/login.");
        };
        uint32_t id = player_conmap[c];
        bool found = false;
        for (auto &r : created_room) {
          if (r->state == ROOM_RUNNING || r->player_len <= 0)
            continue;
          for (int j = 0; j < 2; j++) {
            if (r->players[j]) {
              if (r->players[j]->id == id)
                found = true;
            }
          }
          if (found)
            break;
        }
        if (found) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::CREATE_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Already in room cannot make a new room.");
          break;
        }

        Player *p = &server->players[id];
        if (p) {
          p->health = MAX_PLAYER_HEALTH;
          p->ready = false;
        }
        Room *r = find_free_room(server);
        if (!r) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::CREATE_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Max rooms count reached");
        } else {
          *r = Room{}; // this is the free selected room
          int id = player_conmap[c];
          r->player_len = 1;
          r->players[0] = &server->players[id];
          r->state = ROOM_ACTIVE;
          char *stuff = generate_random_id(
              ID_MAX_COUNT); // NOTE: No need to free its using the
                             // Helper static buffer
          strncpy(r->id, stuff, ID_MAX_COUNT);
          r->id[ID_MAX_COUNT - 1] = 0;
          created_room.push_back(r); // NOTE: Store the room globally
                                     // to have easy access later

          reply.type = MessageType::HERE_ROOM;
          reply.response = MessageType::CREATE_ROOM;
          reply.data.Room_obj = r;
        }
      } break;
      case CONNECT_ROOM: {
        char *msgdata = pd.data.String;
        if (!msgdata)
          break;

        if (!player_conmap.count(c)) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::CONNECT_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Please do GIVE_ID first.");
          break;
        }

        uint32_t my_id = player_conmap[c];
        Room *selroom = nullptr;

        for (auto &r : created_room) {
          if (strncmp(r->id, msgdata, ID_MAX_COUNT) == 0) {
            selroom = r;
            break;
          }
        }

        if (!selroom || selroom->state == ROOM_RUNNING ||
            selroom->player_len >= 2) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::CONNECT_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Room full or not found.");
        } else {
          bool already_in = false;
          for (int j = 0; j < 2; j++) {
            if (selroom->players[j] != nullptr &&
                selroom->players[j]->id == my_id) {
              already_in = true;
              break;
            }
          }

          if (already_in) {
            reply.type = MessageType::ERROR;
            reply.response = MessageType::CONNECT_ROOM;
            snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                     "You are already in this room.");
          } else {
            int slot = get_room_player_empty(selroom);
            if (slot != -1) {
              Player *player = &server->players[my_id];
              player->health = MAX_PLAYER_HEALTH;
              player->ready = false;
              selroom->players[slot] = player;
              selroom->player_len++;

              reply.type = MessageType::HERE_ROOM;
              reply.response = MessageType::CONNECT_ROOM;
              reply.data.Room_obj = selroom;
            } else {
              reply.type = MessageType::ERROR;
              reply.response = MessageType::CONNECT_ROOM;
              snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                       "No empty slots.");
            }
          }
        }
      } break;
      case EXIT_ROOM: {
        if (!player_conmap.count(c)) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::EXIT_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Please do GIVE_ID first to register/login.");
          break;
        }
        uint32_t id = player_conmap[c];
        Room *r = nullptr;
        int idx = -1;
        int roomi = -1;
        bool done = false;
        for (size_t i = 0; i < created_room.size(); i++) {
          Room *ri = created_room[i];
          if (ri->player_len < 1 || ri->player_len > 2)
            continue;
          for (int x = 0; x < 2; x++) {
            if (ri->players[x] && ri->players[x]->id == id) {
              r = ri;
              idx = x;
              roomi = i;
              done = true;
              Player *player = ri->players[x];
              player->health = MAX_PLAYER_HEALTH;
              player->ready = false;
              break;
            }
          }
          if (done)
            break;
        }
        if (r) {
          r->players[idx] = nullptr;
          r->player_len--;
          if (r->player_len <= 0) {
            *r = Room{};
            r->state = ROOM_FREE;
            if (roomi >= 0)
              created_room.erase(created_room.begin() + roomi);
          }

          if (r->player_len == 1 && r->state == RoomState::ROOM_RUNNING) {
            r->state = RoomState::ROOM_ACTIVE;
            Player *op = r->players[idx ^ 1];

            Message newmsg = {};
            newmsg.type = MessageType::GAME_FINISHED_PREMATURELY;
            newmsg.response = MessageType::NONE;
            if (op && op->con)
              ws_send(op->con, &newmsg);
          }

          reply.type = MessageType::EXIT_ROOM;
          reply.response = MessageType::EXIT_ROOM;
          break;
        }
        reply.type = MessageType::ERROR;
        reply.response = MessageType::EXIT_ROOM;
        snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                 "Cannot find the room!");
      } break;
      case GAME_PLAYER_UPDATE: {
        if (!player_conmap.count(c)) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::GAME_PLAYER_UPDATE;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Please do GIVE_ID first to register/login.");
          break;
        }

        uint32_t id = player_conmap[c];
        Room *r = nullptr;
        Player *p = nullptr;
        Player *op = nullptr;
        bool done = false;

        // Find room and players
        for (size_t i = 0; i < created_room.size(); i++) {
          Room *ri = created_room[i];
          if (ri->state != ROOM_RUNNING && ri->player_len < 2)
            continue;
          for (int x = 0; x < 2; x++) {
            if (ri->players[x] && ri->players[x]->id == id) {
              r = ri;
              p = r->players[x];
              op = r->players[x ^ 1];
              done = true;
              break;
            }
          }
          if (done)
            break;
        }

        // If we can't find a valid room/players, send error and bail
        if (!r || !p || !op) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::GAME_PLAYER_UPDATE;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Cannot find the room or opponent.");
          break;
        }
        if (r->turn != p->turn) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::GAME_PLAYER_UPDATE;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Not the player turn to do anything");
          break;
        }

        bool action_processed = false;
        GameAction action = {};
        memcpy(&action, pd.data.action, sizeof(GameAction));
        delete pd.data.action; // dont want to bother with this...
        switch (action.type) {
        case GameActionType::INJECT: {
          int nid = action.data;
          for (auto &n : r->needles) {
            if (n.id == nid) {
              if (n.used) {
                break;
              }
              n.used = true;
              if (n.type == 1) {
                bool used_booster = (p->pe.type == 0 && !p->pe.used);
                int count = used_booster ? 1 * p->pe.data : 1;
                if (used_booster) {
                  // NOTE: reset the booster items 1 time use
                  p->pe.used = true;
                }
                p->health -= count;
                if (p->health <= 0) {
                  // NOTE: you are losing son.
                  r->state = ROOM_ACTIVE;
                  reply = {};
                  reply.type = GAME_END;
                  reply.response = GAME_PLAYER_UPDATE;
                  reply.data.Int = op->id;
                  if (p->con)
                    ws_send(p->con, &reply);
                  if (op->con)
                    ws_send(op->con, &reply);

                  p->ready = false;
                  op->ready = false;
                  reply.type = MessageType::READY_STATUS;
                  reply.response = MessageType::TOGGLE_READY;
                  reply.data.Boolean = (uint8_t)p->ready;
                  ws_send(p->con, &reply);

                  reply.data.Boolean = (uint8_t)op->ready;
                  ws_send(op->con, &reply);
                  return;
                }
              }

              // NOTE: generate the new needle when all used up.
              int used_counter = 0;
              for (const auto &n : r->needles) {
                if (n.used)
                  ++used_counter;
              }
              if (used_counter >= 5) {
                // NOTE: right now it will be filled again until full but if want to add only once
                // we can add break.
                for (int a = 0; a < PLAYER_MAX_ITEMS_COUNT; a++) {
                  if (p->items[a].used) {
                    p->items[a].type = rand_range(0, 1);
                    p->items[a].used = false;
                  }
                }

                for (int a = 0; a < PLAYER_MAX_ITEMS_COUNT; a++) {
                  if (op->items[a].used) {
                    op->items[a].type = rand_range(0, 1);
                    op->items[a].used = false;
                  }
                }
                reply = {};
                reply.type = GAME_ITEMS_INFO;
                reply.response = NONE;

                reply.data.Byte.len = sizeof(MinimalItems) * PLAYER_MAX_ITEMS_COUNT;
                memcpy(reply.data.Byte.data, p->items, reply.data.Byte.len);
                ws_send(p->con, &reply);

                memcpy(reply.data.Byte.data, op->items, reply.data.Byte.len);
                ws_send(op->con, &reply);

                //-----------------

                const int needle_count = 5;
                const int live_needles = rand_range(1, 4);

                std::vector<MinimalNeedle> mn =
                  initialize_needle(needle_count, live_needles, r);

                Message needle_msg = {};
                needle_msg.type = GAME_NEEDLE_DATA;
                needle_msg.response = NONE;
                needle_msg.data.Byte.len = sizeof(MinimalNeedle) * needle_count;
                memcpy(needle_msg.data.Byte.data, r->needles.data(),
                       needle_msg.data.Byte.len);

                if (p->con)
                  ws_send(p->con, &needle_msg);
                if (op->con)
                  ws_send(op->con, &needle_msg);
              }

              reply = {};
              reply.type = PLAYER_INFO;
              reply.response = GAME_PLAYER_UPDATE;
              reply.data.Player_obj = op;
              if (p->con)
                ws_send(p->con, &reply);
              if (op->con)
                ws_send(op->con, &reply);

              reply.data.Player_obj = p;
              if (p->con)
                ws_send(p->con, &reply);
              if (op->con)
                ws_send(op->con, &reply);

              reply = {};
              MinimalNeedle mn = {n.id, n.used};
              reply.type = GAME_NEEDLE_DATA;
              reply.response = GAME_PLAYER_UPDATE;
              reply.data.Byte.len = sizeof(MinimalNeedle);
              memcpy(reply.data.Byte.data, &mn, reply.data.Byte.len);
              if (p->con)
                ws_send(p->con, &reply);
              if (op->con)
                ws_send(op->con, &reply);

              action_processed = true;
              break;
            }
          }
        } break;
        case GameActionType::USE_ITEM: {
          MinimalItems *mn = nullptr;
          printf("get the id args: %d\n", action.data);
          for (auto &it: p->items) {
            if (it.shared_id == action.data) {
              if (it.used) {
                reply.type = MessageType::ERROR;
                reply.response = MessageType::GAME_PLAYER_UPDATE;
                snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                         "Items with that id already used.");
                ws_send(p->con, &reply);
                return;
              }
              mn = &it;
              break;
            }
          }
          if (!mn) {
            reply.type = MessageType::ERROR;
            reply.response = MessageType::GAME_PLAYER_UPDATE;
            snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                     "Items with that id doesn't exist.");
            ws_send(p->con, &reply);
            return;
          }

          mn->used = true;
          reply.type = GAME_ITEMS_INFO;
          reply.response = GAME_TURN_UPDATE;
          reply.data.Byte.len = sizeof(MinimalItems) * PLAYER_MAX_ITEMS_COUNT;
          memcpy(reply.data.Byte.data, p->items, reply.data.Byte.len);
          ws_send(p->con, &reply);

          reply = {};
          switch (mn->type) {
          case 0: { // booster
            p->pe = {
              .type = 0,
              .data = 2,
              .used = false,
            };
          } break;
          case 1: { // revealer
            _MinimalNeedle *n = nullptr;
            for (size_t a = 0; a < r->needles.size(); a++) {
              _MinimalNeedle *in = &r->needles[a];
              if (in && in->type == 1 && !in->used && !in->revealed) {
                in->revealed = true;
                n = in;
                break;
              }
            }
            if (n) {
              reply.type = GAME_REVEALED_ITEMS;
              reply.response = GAME_PLAYER_UPDATE;
              reply.data.Int = n->id; // the needle with this id is live guys
              ws_send(p->con, &reply);
              return;
            } else {
              reply.type = ERROR;
              reply.response = GAME_PLAYER_UPDATE;
              snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                       "Cannot found any live needles, there is something so wrong right now.");
            }
            // NOTE: dont need to do `action_processed = true` because we dont want to flip the turn
          } break;
          default: break;
          }
        } break;
        }
        if (action_processed) {
          r->turn = (PlayerState)((int)r->turn == 1 ? 0 : 1);
          reply.type = MessageType::GAME_TURN_UPDATE;
          reply.response = MessageType::GAME_PLAYER_UPDATE;
          reply.data.Int = (int)r->turn;
          if (p->con)
            ws_send(p->con, &reply);
          if (op->con)
            ws_send(op->con, &reply);
        }
        return;
      } break;
      case TOGGLE_READY: {
        Room *r = nullptr;
        Player *p = nullptr;
        Player *op = nullptr;
        uint32_t id = player_conmap[c];
        for (size_t i = 0; i < created_room.size(); i++) {
          Room *ri = created_room[i];
          if (ri->state != ROOM_ACTIVE)
            continue;
          for (int x = 0; x < 2; x++) {
            if (!ri->players[x])
              continue;
            if (ri->players[x]->id == id) {
              r = ri;
              p = ri->players[x];
              int other_idx = x ^ 1;
              if (ri->players[other_idx] != nullptr) {
                op = ri->players[other_idx];
              }
              break;
            }
          }
          if (r)
            break;
        }
        if (r && p) {
          p->ready = !p->ready; // Toggle ready state
          p->health = MAX_PLAYER_HEALTH;
          if (op) {
            if (op->ready && p->ready) {
              p->turn = PlayerState::PLAYER1;
              op->turn = PlayerState::PLAYER2;

              r->state = ROOM_RUNNING;
              reply.type = MessageType::GAME_START;
              reply.response = MessageType::TOGGLE_READY;

              if (p->con)
                ws_send(p->con, &reply);
              if (op->con)
                ws_send(op->con, &reply);

              // NOTE: player status
              reply = {};
              reply.type = MessageType::PLAYER_INFO;
              reply.response = MessageType::TOGGLE_READY;
              reply.data.Player_obj = p;
              if (p->con)
                ws_send(op->con, &reply);
              if (op->con)
                ws_send(p->con, &reply);

              reply.data.Player_obj = op;
              if (p->con)
                ws_send(p->con, &reply);
              if (op->con)
                ws_send(op->con, &reply);

              // NOTE: Room turn
              reply = {};
              reply.type = MessageType::GAME_TURN_UPDATE;
              reply.response = MessageType::TOGGLE_READY;
              reply.data.Int = (int)r->turn;
              if (p->con)
                ws_send(p->con, &reply);
              if (op->con)
                ws_send(op->con, &reply);

              // NOTE: items
              for (int a = 0; a < PLAYER_MAX_ITEMS_COUNT; a++) {
                p->items[a] = {
                  .shared_id = a,
                  .type = rand_range(0, 1),
                  .used = false,
                };
              }

              for (int a = 0; a < PLAYER_MAX_ITEMS_COUNT; a++) {
                op->items[a] = {
                  .shared_id = a,
                  .type = rand_range(0, 1),
                  .used = false,
                };
              }
              reply = {};
              reply.type = GAME_ITEMS_INFO;
              reply.response = NONE;

              reply.data.Byte.len = sizeof(MinimalItems) * PLAYER_MAX_ITEMS_COUNT;
              memcpy(reply.data.Byte.data, p->items, reply.data.Byte.len);
              ws_send(p->con, &reply);

              memcpy(reply.data.Byte.data, op->items, reply.data.Byte.len);
              ws_send(op->con, &reply);

              // NOTE: needle
              const int needle_count = 5;
              const int live_needles = rand_range(1, 4);

              std::vector<MinimalNeedle> mn =
                  initialize_needle(needle_count, live_needles, r);

              Message needle_msg = {};
              needle_msg.type = GAME_NEEDLE_DATA;
              needle_msg.response = NONE;
              needle_msg.data.Byte.len = sizeof(MinimalNeedle) * needle_count;
              memcpy(needle_msg.data.Byte.data, r->needles.data(),
                     needle_msg.data.Byte.len);
              if (p->con)
                ws_send(p->con, &needle_msg);
              if (op->con)
                ws_send(op->con, &needle_msg);

              return;
            }
          }
          reply.type = MessageType::READY_STATUS;
          reply.response = MessageType::TOGGLE_READY;
          reply.data.Boolean = (uint8_t)p->ready;
          break;
        }
        reply.type = MessageType::ERROR;
        reply.response = MessageType::TOGGLE_READY;
        snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                 "Cannot find the room!");
      } break;
      default: {
      } break;
      }
      ws_send(c, &reply);
    }
  } break;
  case MG_EV_OPEN:
    printf("[SERVER] Connection opened:  %p\n", c);
    break;
  case MG_EV_CLOSE: {
    printf("[SERVER] Connection closed: %p\n", c);
    uint32_t id = player_conmap[c];
    Room *r = nullptr;
    int idx = -1;
    int roomi = -1;
    bool done = false;
    for (size_t i = 0; i < created_room.size(); i++) {
      Room *ri = created_room[i];
      if (ri->player_len < 1 || ri->player_len > 2)
        continue;
      for (int x = 0; x < 2; x++) {
        if (ri->players[x] && ri->players[x]->id == id) {
          r = ri;
          idx = x;
          roomi = i;
          done = true;
          Player *player = ri->players[x];
          player->health = MAX_PLAYER_HEALTH;
          player->ready = false;
          break;
        }
      }
      if (done)
        break;
    }
    if (r) {
      r->players[idx] = nullptr;
      r->player_len--;
      if (r->player_len <= 0) {
        *r = Room{};
        r->state = ROOM_FREE;
        if (roomi >= 0)
          created_room.erase(created_room.begin() + roomi);
      }

      if (r->player_len == 1 && r->state == RoomState::ROOM_RUNNING) {
        r->state = RoomState::ROOM_ACTIVE;
        Player *op = r->players[idx ^ 1];

        Message newmsg = {};
        newmsg.type = MessageType::GAME_FINISHED_PREMATURELY;
        newmsg.response = MessageType::NONE;
        if (op && op->con)
          ws_send(op->con, &newmsg);
      }
      return;
    }

    // NOTE: we dont need to cleanup the big player array right since they
    // should beable to login
    if (player_conmap.count(c)) {
      int id = player_conmap[c];
      server->players[id].con = nullptr;
      player_conmap.erase(c);
    }
  } break;
  case MG_EV_ERROR:
    printf("[SERVER] Error: %s\n", (char *)ev_data);
    break;
  default:
    // NOTE: Dont care about other msg
    break;
  }
}

int main(int argc, char **argv) {
  srand(time(0));
  static const char *ipflag = "-ip";
  static const char *portflag = "-port";
  static const int resolution = 200;

  bool error = false;
  std::string ip = "0.0.0.0";
  uint16_t port = 8000;

  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], ipflag, strlen(ipflag)) == 0 && i + 1 < argc)
      ip = argv[i++];
    else if (strncmp(argv[i], portflag, strlen(portflag)) == 0 &&
             i + 1 < argc) {
      char *_now = argv[i++];
      int convert = atoi(_now);
      if (convert <= 0) {
        fprintf(stderr, "ERROR: Invalid port `%s`.\n", _now);
        error = true;
        break;
      }
      port = (uint16_t)convert;
    }
  }
  if (error)
    return 1;

  Server s(ip.c_str(), port, ws_handler);
  s.add_timer(resolution, MG_TIMER_REPEAT, timer_fn, &s.mgr);
  s.loop(resolution);
  return 0;
}

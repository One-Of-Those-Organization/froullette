#define _SERVER

#include "Server.hpp"
#include "../Message/Message.hpp"
#include "../Shared/Helper.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

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

static void timer_fn(void *arg) {
  (void)arg;
  for (auto &r : created_room) {
    Player *p = r->players[0];
    Player *op = r->players[1];
    if (!p || !op)
      continue; // if only 1 available continue to the next room skip the
                // current one, take only the room with 2 valid player
    if (r->state == ROOM_ACTIVE) {
      Message msg = {};
      msg.type = LOBBY_STATUS;
      msg.response = NONE;
      msg.data.LobbyStatus_obj = {r->player_len, {p->ready, op->ready}};
      if (p->con)  ws_send(p->con, &msg);
      if (op->con) ws_send(op->con, &msg);
    }
    /*
    else if (r->state == ROOM_RUNNING) {
      Message msg = {};
      msg.type = GAME_PERIODIC;
      msg.response = NONE;
    }
    */
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
            .ready = false,
            .turn = PlayerState::PLAYER1, // NOTE: update this on room
                                          // enter.
        };
        player_conmap[c] = server->ccount;
        ++server->ccount;
      } break;
      case CREATE_ROOM: {
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
              selroom->players[slot] = &server->players[my_id];
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
            if (roomi >= 0)
              created_room.erase(created_room.begin() + roomi);
          }

          if (r->player_len == 1 && r->state == RoomState::ROOM_RUNNING) {
            r->state = RoomState::ROOM_ACTIVE;
            Player *op = r->players[idx ^ 1];

            Message newmsg = {};
            newmsg.type = MessageType::GAME_FINISHED_PREMATURELY;
            newmsg.response = MessageType::NONE;
            if (op->con) ws_send(op->con, &newmsg);
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
      // TODO: finish this
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
          if (ri->player_len < 2) continue;
          for (int x = 0; x < 2; x++) {
            if (ri->players[x] && ri->players[x]->id == id) {
              r = ri;
              p = r->players[x];
              op = r->players[x ^ 1];
              done = true;
              break;
            }
          }
          if (done) break;
        }

        // If we can't find a valid room/players, send error and bail
        if (!r || !p || !op) {
          reply.type = MessageType::ERROR;
          reply.response = MessageType::GAME_PLAYER_UPDATE;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                   "Cannot find the room or opponent.");
          break;
        }

        bool action_processed = false;

        switch (pd.data.action->type) {
        case INJECT: {
          int nid = pd.data.action->data.i32;
          for (auto &n : r->needles) {
            if (n.id == nid) {
              if (n.used) break; // Already used, ignore

              n.used = true;
              if (n.type == 1) {
                p->health--;
              }

              uint8_t out[MAX_MESSAGE_BIN_SIZE];

              // send opponent info to me
              reply.type = PLAYER_INFO;
              reply.response = GAME_PLAYER_UPDATE;
              reply.data.Player_obj = op;
              if (p->con) ws_send(p->con, &reply);

              // send my info to opponent
              reply.data.Player_obj = p;
              if (op->con) ws_send(op->con, &reply);

              // send Needle state to both
              memset(out, 0, MAX_MESSAGE_BIN_SIZE);
              reply = {};
              MinimalNeedle mn = {n.id, n.used};
              reply.type = GAME_NEEDLE_DATA;
              reply.response = GAME_PLAYER_UPDATE;
              reply.data.Byte.len = sizeof(MinimalNeedle);
              memcpy(reply.data.Byte.data, &mn, reply.data.Byte.len);
              if (p->con) ws_send(p->con, &reply);
              if (op->con) ws_send(op->con, &reply);

              action_processed = true;
              break;
            }
          }
        } break;
        case USE_ITEM: {
          int nid = pd.data.action->data.i32;
          (void)nid;
          // TODO: implement
          action_processed = false;
        } break;
        }

        // Only flip turn + broadcast turn update if an action actually happened
        if (action_processed) {
          r->turn = (PlayerState)((int)r->turn == 1 ? 0 : 1);
          reply.type = MessageType::GAME_TURN_UPDATE;
          reply.response = MessageType::GAME_PLAYER_UPDATE;
          reply.data.Boolean = (uint8_t)r->turn;

          if (p->con)  ws_send(p->con, &reply);
          if (op->con) ws_send(op->con, &reply);
        }

        return;
      };
      case TOGGLE_READY: {
        Room *r = nullptr;
        Player *p = nullptr;
        Player *op = nullptr;
        uint32_t id = player_conmap[c];
        for (size_t i = 0; i < created_room.size(); i++) {
          Room *ri = created_room[i];
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
          if (op) {
            if (op->ready && p->ready) {
              p->turn = PlayerState::PLAYER1;
              op->turn = PlayerState::PLAYER2;

              r->state = ROOM_RUNNING;
              reply.type = MessageType::GAME_START;
              reply.response = MessageType::TOGGLE_READY;

              if (p->con)  ws_send(p->con, &reply);
              if (op->con) ws_send(op->con, &reply);

              // NOTE: player status
              reply = {};
              reply.type = MessageType::PLAYER_INFO;
              reply.response = MessageType::TOGGLE_READY;
              reply.data.Player_obj = p;
              if (p->con)  ws_send(op->con, &reply);

              reply.data.Player_obj = op;
              if (op->con) ws_send(p->con, &reply);

              // NOTE: Room turn
              reply = {};
              reply.type = MessageType::GAME_TURN_UPDATE;
              reply.response = MessageType::TOGGLE_READY;
              reply.data.Boolean = (uint8_t)r->turn;
              if (p->con)  ws_send(p->con, &reply);
              if (op->con) ws_send(op->con, &reply);

              // TODO: make this configurable from the outside
              const int needle_count = 5;
              const int live_needles = 2;
              std::vector<uint8_t> needle_types;
              for (int i = 0; i < live_needles; ++i)
                needle_types.push_back(1);
              for (int i = 0; i < needle_count - live_needles; ++i)
                needle_types.push_back(0);

              unsigned seed =
                  std::chrono::system_clock::now().time_since_epoch().count();
              std::shuffle(needle_types.begin(), needle_types.end(),
                           std::default_random_engine(seed));

              std::vector<MinimalNeedle> mn;
              mn.reserve(needle_count);
              r->needles.clear();

              // NOTE: plaese sync the id with the client right now it is since
              // the 2 of them use 0..4
              for (int i = 0; i < needle_count; ++i) {
                _MinimalNeedle needle = {(uint8_t)i, false, needle_types[i]};
                r->needles.push_back(needle);
                mn.push_back({needle.id, needle.used});
              }

              Message needle_msg = {};
              needle_msg.type = GAME_NEEDLE_DATA;
              needle_msg.response = NONE;
              needle_msg.data.Byte.len = sizeof(MinimalNeedle) * needle_count;
              memcpy(needle_msg.data.Byte.data, r->needles.data(),
                     needle_msg.data.Byte.len);
              if (p->con)  ws_send(p->con, &needle_msg);
              if (op->con) ws_send(op->con, &needle_msg);

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
  case MG_EV_CLOSE:
    printf("[SERVER] Connection closed: %p\n", c);
    // TODO: cleanup their room if no one there.
    // NOTE: we dont need to cleanup the big player array right since they
    // should beable to login
    if (player_conmap.count(c)) {
      int id = player_conmap[c];
      server->players[id].con = nullptr;
      player_conmap.erase(c);
    }
    break;
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
  static const int resolution = 100;

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

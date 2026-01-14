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

Room *find_free_room(Server *server) {
  for (size_t i = 0; i < MAX_ROOM_COUNT; i++) {
    if (server->rooms[i].state == ROOM_FREE) {
      return &server->rooms[i];
    }
  }
  return nullptr;
}

static void timer_fn(void *arg) {
  uint8_t out[MAX_MESSAGE_BIN_SIZE];
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
      memset(out, 0, MAX_MESSAGE_BIN_SIZE);
      size_t n = generate_network_field(&msg, out);
      printf("Generated data with size: %zu\n", n);
      mg_ws_send(p->con, out, n, WEBSOCKET_OP_BINARY);
      mg_ws_send(op->con, out, n, WEBSOCKET_OP_BINARY);
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
        for (size_t i = 0; i < created_room.size(); i++) {
          Room *ri = created_room[i];
          if (ri->player_len < 1 || ri->player_len > 2)
            continue;
          for (int x = 0; x < 2; x++) {
            if (ri->players[x] && ri->players[x]->id == id) {
              r = ri;
              idx = x;
              roomi = i;
              break;
            }
          }
        }
        if (r) {
          r->players[idx] = nullptr;
          r->player_len--;
          if (r->player_len <= 0) {
            *r = Room{};
            if (roomi >= 0)
              created_room.erase(created_room.begin() + roomi);
          }

          // NOTE: if other player leave and the room is running auto change the room state
          // since they now will play with ghsost if we continue.
          if (r->player_len == 1 && r->state == RoomState::ROOM_RUNNING) {
              int other_idx = idx ^ 1;
              r->state = RoomState::ROOM_ACTIVE;
              Player *op = r->players[other_idx];

              Message newmsg = {};
              newmsg.type = MessageType::GAME_FINISHED_PREMATURELY;
              newmsg.response = MessageType::NONE;

              uint8_t inside_out[MAX_MESSAGE_BIN_SIZE];
              size_t n = generate_network_field(&newmsg, inside_out);
              printf("Generated data with size: %zu\n", n);
              mg_ws_send(op->con, inside_out, n, WEBSOCKET_OP_BINARY);
          }

          reply.type = MessageType::NONE;
          reply.response = MessageType::EXIT_ROOM;
          snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE, "Done removed!");
          break;
        }
        reply.type = MessageType::ERROR;
        reply.response = MessageType::EXIT_ROOM;
        snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                 "Cannot find the room!");
      } break;
      case GAME_PLAYER_UPDATE: {
        // Search for the room and player
        uint32_t id = player_conmap[c];
        Room *r = nullptr;
        Player *p = nullptr;
        Player *op = nullptr;

        for (auto &ri : created_room) {
          if (ri->state != ROOM_RUNNING) continue;
          for (int x = 0; x < 2; x++) {
            if (ri->players[x] && ri->players[x]->id == id) {
              r = ri;
              p = ri->players[x];
              op = ri->players[x ^ 1];
              break;
            }
          }
          if (r) break;
        }

        if (!r || !p || !op) break;

        // Read from Player State and from Server || Turn Logic
        if (r->turn == PlayerState::PLAYER1 && p != r->players[0]) break; // Player 1 Turn
        if (r->turn == PlayerState::PLAYER2 && p != r->players[1]) break; // Player 2 Turn

        // Process Action
        switch (pd.data.action->type) {
          case INJECT: {
            int needle_idx = pd.data.action->data.i32;

            // Validate needle index
            if (needle_idx < 0 || needle_idx >= (int)r->needles.size()) break; // Prevent out of index range
            if (r->needles[needle_idx].used) break; // Break the needles that already used

            // Update Needles
            r->needles[needle_idx].used = true;
            bool is_live = (r->needles[needle_idx].type == 1); // LIVE NEEDLES

            // Broadcast to Clients about the needle update
            {
              Message needle_msg = {};
              needle_msg.type = GAME_NEEDLE_DATA;
              needle_msg.response = NONE;

              std::vector<MinimalNeedle> minimal_needles;
              for (auto &n : r->needles) minimal_needles.push_back({n.id, n.used});

              needle_msg.data.Byte.len = sizeof(MinimalNeedle) * minimal_needles.size();
              memcpy(needle_msg.data.Byte.data, minimal_needles.data(), needle_msg.data.Byte.len);

              uint8_t out[MAX_MESSAGE_BIN_SIZE];
              size_t n = generate_network_field(&needle_msg, out);
              mg_ws_send(p->con, out, n, WEBSOCKET_OP_BINARY);
              mg_ws_send(op->con, out, n, WEBSOCKET_OP_BINARY);
            }

            // Process Injection
            if (is_live) {
              p->health -= 1; // Give damage to self player
              r->turn = (r->turn == PlayerState::PLAYER1) ? PlayerState::PLAYER2 : PlayerState::PLAYER1;
            } else {
              // NOTE : For now skip turn only and not having more turn
              r->turn = (r->turn == PlayerState::PLAYER1) ? PlayerState::PLAYER2 : PlayerState::PLAYER1;
            }

            // Game Over State
            if (p->health <= 0) {
              r->state = ROOM_FINISHED;
              // TODO : Finish Game Over State
            }

            // Check for Next Round
            bool all_used = true;
            for (auto &n : r->needles) {
              if (!n.used) {
                all_used = false;
                break;
              }
            }

            if (all_used && r->state != ROOM_FINISHED) {
              printf("[Server.cpp line %d] All needles used, resetting needles for next round.\n", __LINE__);

              // Clean old needles
              r->needles.clear();

              // Generate new needles
              const int needle_count = 5;
              const int live_needles = rand() % 3 + 1; // Random between 1 to 3 live needles

              std::vector<uint8_t> needle_types;
              for (int i = 0; i < live_needles; ++i) needle_types.push_back(1);
              for (int i = 0; i < needle_count - live_needles; ++i) needle_types.push_back(0);

              unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
              std::shuffle(needle_types.begin(), needle_types.end(), std::default_random_engine(seed));

              // Send to both players
              for (int i = 0; i < needle_count; ++i) {
                _MinimalNeedle needle = {(uint8_t)i, false, needle_types[i]};
                r->needles.push_back(needle);
              }

              // After reset needles now send to both players
              Message needle_msg = {};
              needle_msg.type = GAME_NEEDLE_DATA;
              needle_msg.response = NONE;

              std::vector<MinimalNeedle> mn;
              for(auto &n : r->needles) mn.push_back({n.id, n.used});

              needle_msg.data.Byte.len = sizeof(MinimalNeedle) * mn.size();
              memcpy(needle_msg.data.Byte.data, mn.data(), needle_msg.data.Byte.len);

              uint8_t out_reset[MAX_MESSAGE_BIN_SIZE];
              size_t n_reset = generate_network_field(&needle_msg, out_reset);
              mg_ws_send(p->con, out_reset, n_reset, WEBSOCKET_OP_BINARY);
              mg_ws_send(op->con, out_reset, n_reset, WEBSOCKET_OP_BINARY);
            }

            // Send Game Start Update (Contain HP, etc)
            Message status_msg = {};
            status_msg.type = GAME_START;
            Player buffer[] = {*r->players[0], *r->players[1]};
            status_msg.data.Byte.len = sizeof(Player) * 2;
            memcpy(status_msg.data.Byte.data, buffer, sizeof(Player) * 2);

            uint8_t out[MAX_MESSAGE_BIN_SIZE];
            size_t n = generate_network_field(&status_msg, out);
            mg_ws_send(p->con, out, n, WEBSOCKET_OP_BINARY);
            mg_ws_send(op->con, out, n, WEBSOCKET_OP_BINARY);

            // Send Turn Update
            Message turn_msg = {};
            turn_msg.type = GAME_TURN_UPDATE;
            turn_msg.response = NONE;
            turn_msg.data.Boolean = (uint8_t)r->turn;

            size_t n_turn = generate_network_field(&turn_msg, out);
            mg_ws_send(p->con, out, n_turn, WEBSOCKET_OP_BINARY);
            mg_ws_send(op->con, out, n_turn, WEBSOCKET_OP_BINARY);
          } break;
          case USE_ITEM: {
            // TODO : Finish Use Item
          } break;
        }
      } break;
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
              Player buffer[] = {*p, *op};
              r->state = ROOM_RUNNING;
              reply.type = MessageType::GAME_START;
              reply.response = MessageType::TOGGLE_READY;
              reply.data.Byte.len = sizeof(Player) * 2;
              memcpy(reply.data.Byte.data, buffer, reply.data.Byte.len);

              // NOTE: send to other player too
              uint8_t out[MAX_MESSAGE_BIN_SIZE];
              size_t n = generate_network_field(&reply, out);
              printf("Generated data with size: %zu\n", n);
              mg_ws_send(p->con, out, n, WEBSOCKET_OP_BINARY);
              mg_ws_send(op->con, out, n, WEBSOCKET_OP_BINARY);

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

              // NOTE: plaese sync the id with the client right now it is since the 2 of them use 0..4
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

              uint8_t needle_out[MAX_MESSAGE_BIN_SIZE];
              size_t needle_n = generate_network_field(&needle_msg, needle_out);
              printf("Generated data with size: %zu\n", needle_n);
              mg_ws_send(p->con, needle_out, needle_n, WEBSOCKET_OP_BINARY);
              mg_ws_send(op->con, needle_out, needle_n, WEBSOCKET_OP_BINARY);

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

      uint8_t out[MAX_MESSAGE_BIN_SIZE];
      size_t n = generate_network_field(&reply, out);
      printf("Generated data with size: %zu\n", n);
      mg_ws_send(c, out, n, WEBSOCKET_OP_BINARY);
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

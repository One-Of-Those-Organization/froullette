#define _SERVER

#include "Server.hpp"
#include "../Message/Message.hpp"
#include "../Shared/Helper.hpp"

static std::unordered_map<mg_connection*, uint32_t> player_conmap = {};
static std::vector<Room *> created_room = {};

Room *find_free_room(Server* server) {
    for (size_t i = 0; i < MAX_ROOM_COUNT; i++) {
        if (server->rooms[i].state == ROOM_FREE) {
            return &server->rooms[i];
        }
    }
    return nullptr;
}

static void timer_fn(void *arg)
{
    (void)arg;
    /*
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    for (struct mg_connection *wc = mgr->conns; wc != NULL; wc = wc->next) {
        if (wc->data[0] == 'W') {
        }
    }
    */
    for (auto &r: created_room) {
        Player *p = r->players[0];
        Player *op = r->players[1];
        if (!p || !op) continue; // if only 1 available continue to the next room skip the current one, take only the room with 2 valid player

        // TODO: broadcast the player stuff to each other
    }
}

static void ws_handler(mg_connection *c, int ev, void *ev_data)
{
    Server *server = (Server *)c->fn_data;
    switch (ev) {
    case MG_EV_HTTP_MSG: {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/"), NULL)) {
            mg_ws_upgrade(c, hm, NULL);
            c->data[0] = 'W';
        }
        break;
    }
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

            Message reply{};
            switch (pd.type) {
            case GIVE_ID: {
                reply.type = HERE_ID;
                reply.data.Int = server->ccount;
                server->players[server->ccount] = Player{
                    .id = server->ccount,
                    .health = MAX_PLAYER_HEALTH,
                    .con = c,
                    .ready = false
                };
                player_conmap[c] = server->ccount;
                ++server->ccount;
            } break;
            case CREATE_ROOM: {
                uint32_t id = player_conmap[c];
                bool found = false;
                for (auto &r: created_room) {
                    if (r->state == ROOM_RUNNING || r->player_len <= 0) continue;
                    for (int j = 0; j < 2; j++) {
                        if (r->players[j]) {
                            if (r->players[j]->id == id) found = true;
                        }
                    }
                    if (found) break;
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
                    char *stuff = generate_random_id(ID_MAX_COUNT); // NOTE: No need to free its using the Helper static buffer
                    strncpy(r->id, stuff, ID_MAX_COUNT);
                    r->id[ID_MAX_COUNT - 1] = 0;
                    created_room.push_back(r); // NOTE: Store the room globally to have easy access later

                    reply.type = MessageType::HERE_ROOM;
                    reply.response = MessageType::CREATE_ROOM;
                    reply.data.Room_obj = r;
                }
            } break;
            case CONNECT_ROOM: {
                char *msgdata = pd.data.String;
                if (!msgdata) break;

                if (!player_conmap.count(c)) {
                    reply.type = MessageType::ERROR;
                    reply.response = MessageType::CONNECT_ROOM;
                    snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE, "Please do GIVE_ID first.");
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

                if (!selroom || selroom->state == ROOM_RUNNING || selroom->player_len >= 2) {
                    reply.type = MessageType::ERROR;
                    reply.response = MessageType::CONNECT_ROOM;
                    snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE, "Room full or not found.");
                } else {
                    bool already_in = false;
                    for (int j = 0; j < 2; j++) {
                        if (selroom->players[j] != nullptr && selroom->players[j]->id == my_id) {
                            already_in = true;
                            break;
                        }
                    }

                    if (already_in) {
                        reply.type = MessageType::ERROR;
                        reply.response = MessageType::CONNECT_ROOM;
                        snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE, "You are already in this room.");
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
                            snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE, "No empty slots.");
                        }
                    }
                }
            } break;
            case EXIT_ROOM: {
                // TODO: this guy is the one that make empty info log
            } break;
            case GAME_START: {
                Room *r = nullptr;
                Player *p = nullptr;
                Player *op = nullptr;
                uint32_t id = player_conmap[c];
                for (size_t i = 0; i < created_room.size(); i++) {
                    Room *ri = created_room[i];
                    for (int x = 0; x < 2; x++) {
                        if (!ri->players[x]) continue;
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
                    if (r) break;
                }
                if (r && p) {
                    p->ready = !p->ready; // Toggle ready state
                    if (op) {
                        if (op->ready && p->ready) { // if 2 of the player ready then start the game
                            printf("%d and %d ready\n", p->ready, op->ready);
                            r->state = ROOM_RUNNING;
                            reply.type = MessageType::OK;
                            reply.response = MessageType::GAME_START;
                            snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                                "Started the game!");
                            break;
                        }
                    }
                    reply.type = MessageType::READY;
                    reply.response = MessageType::GAME_START;
                    reply.data.Boolean = (uint8_t)p->ready;
                    break;
                }
                reply.type = MessageType::ERROR;
                reply.response = MessageType::GAME_START;
                snprintf(reply.data.String, MAX_MESSAGE_STRING_SIZE,
                        "Cannot find the room!");
            } break;
            default: {} break;
            }

            uint8_t out[MAX_MESSAGE_BIN_SIZE];
            size_t n = generate_network_field(&reply, out);
            printf("Generated data with size: %zu\n", n);
            mg_ws_send(c, out, n, WEBSOCKET_OP_BINARY);
        }
    //     Message msg = {};
    //     switch ((int)msgtype) {
    //     case EXIT_ROOM: {
    //         if (!player_conmap.count(c)) {
    //             msg.type = MessageType::ERROR;
    //             msg.response = MessageType::EXIT_ROOM;
    //             snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
    //                     "Please do GIVE_ID first to register/login.");
    //             break;
    //         }
    //         uint32_t id = player_conmap[c];
    //         Room *r = nullptr;
    //         for (size_t i = 0; i < MAX_ROOM_COUNT; i++) {
    //             Room *ri = &server->rooms[i];
    //             if (ri->player_len < 1 || (ri->players[0]->id != id && ri->players[1]->id != id) || ri->player_len >= 2)
    //             {
    //                 continue;
    //             } else {
    //                 r = ri;
    //                 break;
    //             }
    //         }
    //         if (r) {
    //             int index = get_room_player_empty(r);
    //             if (index < 0) {
    //                 msg.type = MessageType::ERROR;
    //                 msg.response = MessageType::EXIT_ROOM;
    //                 snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
    //                     "UNREACHABLE");
    //                 break;
    //             }
    //             r->players[index] = nullptr;
    //             r->player_len--;
    //             if (r->player_len <= 0) { *r = Room{}; } // NOTE: Reset all of the value

    //             msg.type = MessageType::NONE;
    //             msg.response = MessageType::EXIT_ROOM;
    //             snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
    //                     "Done removed!");
    //             break;
    //         }
    //         msg.type = MessageType::ERROR;
    //         msg.response = MessageType::EXIT_ROOM;
    //         snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
    //                 "Cannot find the room!");
    //     } break;
    //     case GAME_PLAYER_UPDATE: { /* NOTE: change the `turn` here and check first so the one that send this is valid and actually their turn. */ } break;
    //     case GAME_END  : { /* NOTE: The sender of this msg is just server so ignore any message with this. */ } break;
    //     default:
    //         break;
    //     }
    } break;
    case MG_EV_OPEN:
        printf("[SERVER] Connection opened:  %p\n", c);
        break;
    case MG_EV_CLOSE:
        printf("[SERVER] Connection closed: %p\n", c);
        // Clean up player from player_conmap here!
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

int main(int argc, char **argv)
{
    static const char *ipflag = "-ip";
    static const char *portflag = "-port";
    static const int resolution = 100;

    bool error = false;
    std::string ip = "0.0.0.0";
    uint16_t port = 8000;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], ipflag, strlen(ipflag)) == 0 && i + 1 < argc) ip = argv[i++];
        else if (strncmp(argv[i], portflag, strlen(portflag)) == 0 && i + 1 < argc) {
            char *_now = argv[i++];
            int convert = atoi(_now);
            if (convert <= 0) {
                fprintf(stderr, "ERROR: Invalid port `%s`.\n", _now);
                error = true;
                break;
            }
            port = (uint16_t) convert;
        }
    }
    if (error) return 1;

    Server s(ip.c_str(), port, ws_handler);
    s.add_timer(resolution, MG_TIMER_REPEAT, timer_fn, &s.mgr);
    s.loop(resolution);
    return 0;
}

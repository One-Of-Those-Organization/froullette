#include "Server.hpp"
#include "../Message/Message.hpp"
#include "../Shared/Helper.hpp"

static std::unordered_map<mg_connection*, uint32_t> player_conmap = {};

// Flow: hit the server with the port and ip (its http)
//       server send ok with id and pass
//       client save the creds
//       client send make_room or join_room request
//       server response and changing the backstate
//       server send back the room info
//       client send ready
//       server start game and change some state when all ready
//       the game comms

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
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    for (struct mg_connection *wc = mgr->conns; wc != NULL; wc = wc->next) {
      if (wc->data[0] == 'W') {}
        // mg_ws_send(wc, data->buf, data->len, WEBSOCKET_OP_TEXT);
    }
}

static void ws_handler(mg_connection *c, int ev, void *ev_data)
{
    Server *server = (Server *)c->fn_data;
    bool use_bin = false;
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
        struct mg_str payload = wm->data;
        double msgtype;
        bool success = mg_json_get_num(payload, "$.type", &msgtype);
        if (!success) break;

        Message msg = {};
        switch ((int)msgtype) {
        case GIVE_ID: {
            msg.type = MessageType::HERE_ID;
            msg.response = MessageType::GIVE_ID;
            msg.data.Int = server->ccount;
            // save the player
            server->players[server->ccount] = Player{
                .id = server->ccount,
                .health = MAX_PLAYER_HEALTH,
                .con = c,
            };
            player_conmap[c] = server->ccount;
            ++server->ccount;
        } break;
        case CONNECT_ROOM: {
            char *msgdata = mg_json_get_str(payload, "$.data");
            if (!msgdata) break;
            Room *selroom = nullptr;
            for (auto &r: server->rooms) {
                if (strncmp(r.id, msgdata, ID_MAX_COUNT) == 0) {
                    if (r.player_len < 2 && r.state != ROOM_RUNNING) {
                        selroom = &r;
                        break;
                    }
                }
            }
            if (!selroom) {
                msg.type = MessageType::ERROR;
                msg.response = MessageType::CONNECT_ROOM;
                snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
                        "Please send GIVE_ID first to get id.");
                } else {
                    if (!player_conmap.count(c)) {
                        msg.type = MessageType::ERROR;
                        msg.response = MessageType::CREATE_ROOM;
                        snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
                                "Max rooms count reached");
                        break;
                    }
                    use_bin = true;
                    int id = player_conmap[c];
                    selroom->players[selroom->player_len++] = server->players[id];

                    msg.type = MessageType::HERE_ROOM;
                    msg.response = MessageType::CONNECT_ROOM;
                    msg.data.Room_obj = selroom;

                    uint8_t buffer[MAX_MESSAGE_BIN_SIZE] = {};
                    size_t packet_len = generate_network_field(&msg, buffer);
                    mg_ws_send(c, buffer, packet_len, WEBSOCKET_OP_BINARY);
                }
        } break;
        case CREATE_ROOM: {
            Room *r = find_free_room(server);
            if (!r) {
                msg.type = MessageType::ERROR;
                msg.response = MessageType::CREATE_ROOM;
                snprintf(msg.data.String, MAX_MESSAGE_STRING_SIZE,
                        "Max rooms count reached");
            } else {
                use_bin = true;
                *r = Room{}; // this is the free selected room
                int id = player_conmap[c];
                r->players[r->player_len++] = server->players[id];
                r->state = ROOM_ACTIVE;
                char *stuff = generate_random_id(ID_MAX_COUNT); // NOTE: No need to free its using the Helper static buffer
                strncpy(r->id, stuff, ID_MAX_COUNT);
                r->id[ID_MAX_COUNT - 1] = 0;

                msg.type = MessageType::HERE_ROOM;
                msg.response = MessageType::CREATE_ROOM;
                msg.data.Room_obj = r;

                uint8_t buffer[MAX_MESSAGE_BIN_SIZE] = {};
                size_t packet_len = generate_network_field(&msg, buffer);
                mg_ws_send(c, buffer, packet_len, WEBSOCKET_OP_BINARY);
            }
        } break;
        default:
            break;
        }
        if (!use_bin) mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%M", print_msg, &msg);
        break;
    }
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

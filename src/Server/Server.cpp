#include "Server.hpp"
#include "../Message/Message.hpp"

// Flow: hit the server with the port and ip (its http)
//       server send ok with id and pass
//       client save the creds
//       client send make_room or join_room request
//       server response and changing the backstate
//       server send back the room info
//       client send ready
//       server start game and change some state when all ready
//       the game comms

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
    switch (ev) {
    case MG_EV_HTTP_MSG: {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/"), NULL)) {
            mg_ws_upgrade(c, hm, NULL);
            // NOTE: https://mongoose.ws/documentation/tutorials/websocket/websocket-server/
            c->data[0] = 'W';
        }
        break;
    }
    case MG_EV_WS_MSG: {
        mg_ws_message *wm = (mg_ws_message *)ev_data;
        struct mg_str payload = wm->data;
        double msgtype;
        bool success = mg_json_get_num(payload, "$.type", &msgtype);
        if (!success) {
            break;
        }

        Message msg = {};
        switch (msgtype) {
        case GIVE_ID: {
            msg.type = MessageType::HERE_ID;
            msg.response = MessageType::GIVE_ID;
            msg.data.Int = server->ccount++;
        } break;
        case CREATE_ROOM: {
            // TODO
        } break;
        default:
            break;
        }
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "%M", print_msg, &msg);
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

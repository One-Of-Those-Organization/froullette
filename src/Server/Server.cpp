#include <iostream>
#include "mongoose.h"
#include "Server.hpp"

// Flow: hit the server with the port and ip (its http)
//       server send ok with id and pass
//       client save the creds
//       client send make_room or join_room request
//       server response and changing the backstate
//       server send back the room info
//       client send ready
//       server start game and change some state when all ready
//       the game comms

static void ws_handler(mg_connection *c, int ev, void *ev_data)
{
    switch (ev) {
    case MG_EV_WS_MSG: {
        mg_ws_message *wm = (mg_ws_message *)ev_data;
        printf("GET: %.*s\n", (int)wm->data.len, wm->data.buf);
        mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT);
        break;
    }
    default:
        // Dont care about other msg
        break;
    }
}

int main(int argc, char **argv)
{
    static const char *ipflag = "-ip";
    static const char *portflag = "-port";

    bool error = false;
    std::string ip = "0.0.0.0";
    uint16_t port = 8000;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], ipflag, strlen(ipflag)) == 0 && i + 1 < argc) ip = argv[i++];
        else if (strncmp(argv[i], portflag, strlen(portflag)) == 0 && i + 1 < argc) {
            char *_now = argv[i++];
            int convert = atoi(_now);
            if (convert <= 0) {
                std::cerr << "ERROR: Invalid port `" << _now << "`." << std::endl;
                error = true;
                break;
            }
            port = (uint16_t) convert;
        }
    }
    if (error) return 1;

    Server s(ip.c_str(), port, ws_handler);
    s.loop(500);
    return 0;
}

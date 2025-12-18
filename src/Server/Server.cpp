// SERVER: Serve HTTP with Websocket capability.
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

static void ws_handler(mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_WS_MSG) {
        mg_ws_message *wm = (mg_ws_message *) ev_data;
        printf("Received: %.*s\n", (int)wm->data.len, wm->data.buf);
        mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT);
    }
}

int main(void)
{
    Server s("0.0.0.0", 8000u);
    s.callback = ws_handler;
    s.loop(1000);
    return 0;
}

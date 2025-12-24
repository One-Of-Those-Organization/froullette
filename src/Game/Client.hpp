#pragma once
#include "../mongoose.h"

#define DEFAULT_BUFFER_SIZE 1024

class Client {
    public:
        std::string ip;
        uint16_t port;
        void (*callback)(struct mg_connection *, int ev, void *ev_data);
        char _buffer[DEFAULT_BUFFER_SIZE];
        struct mg_mgr mgr;        // Event manager
        bool done = false;        // Event handler flips it to true
        struct mg_connection *c;  // Client connection

        Client();
        ~Client() = default;

        bool connect() {
            mg_mgr_init(&this->mgr);
            snprintf(this->_buffer,
                     DEFAULT_BUFFER_SIZE, "ws://%s:%d",
                     this->ip.c_str(), this->port);
            this->c = mg_ws_connect(&this->mgr, this->_buffer,
                                    this->callback, &this->done, NULL);
            // TODO: Connect to ws
            return false;
        }
};

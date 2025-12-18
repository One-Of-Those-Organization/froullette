#pragma once
#include <iostream>
#include "mongoose.h"

class Server {
    public:
        size_t buffer_size = 64;
        mg_mgr mgr;
        char *buffer = (char *)malloc(buffer_size);
        void (*callback)(mg_connection *c, int ev, void *ev_data);

        Server(const char *ip, uint16_t port) {
            if (snprintf(buffer, buffer_size, "ws://%s:%u", ip, port) < 0) {
                std::cout << "ERROR: Failed to allocate mem" << std::endl;
            }
            mg_mgr_init(&this->mgr);
            mg_http_listen(&this->mgr, buffer, callback, NULL);
        };

        void loop(size_t timeout_ms) {
            for (;;) {
                mg_mgr_poll(&mgr, timeout_ms);
            }
        }

        ~Server() {
            mg_mgr_free(&this->mgr);
            free(this->buffer);
        };
    };

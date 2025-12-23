#pragma once
#include <iostream>
#include "mongoose.h"

#define STR_BUFFER_SIZE 64

class Server {
    public:
        size_t buffer_size = STR_BUFFER_SIZE;
        mg_mgr mgr;
        char *buffer = nullptr;
        void (*callback)(mg_connection *c, int ev, void *ev_data);

        Server(const char *ip, uint16_t port, void (*callback)(mg_connection *c, int ev, void *ev_data)) {
            this->callback = callback;
            this->buffer = (char *)malloc(buffer_size);
            if (!this->buffer) {
                std::cerr << "ERROR: Failed to allocate buffer." << std::endl;
            }
            if (snprintf(buffer, buffer_size, "ws://%s:%u", ip, port) < 0) {
                std::cerr << "ERROR: Failed to built the address string." << std::endl;
            }
            mg_mgr_init(&this->mgr);
            mg_http_listen(&this->mgr, buffer, callback, NULL);
        };

        void add_timer(size_t timeout_ms, int flag, void(*callback)(void *data), void *data){
            mg_timer_add(&this->mgr, timeout_ms, flag, callback, data);
        }

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

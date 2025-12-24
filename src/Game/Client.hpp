#pragma once
#include "../mongoose.h"
#include <atomic>
#include <iostream>

#define DEFAULT_BUFFER_SIZE 1024

class Client {
    public:
        std::string ip;
        uint16_t port;
        void (*callback)(struct mg_connection *, int ev, void *ev_data);
        char _buffer[DEFAULT_BUFFER_SIZE];
        struct mg_mgr mgr;
        std::atomic<bool> done{false};
        struct mg_connection *c = nullptr;

        Client() {};
        ~Client() { this->cleanup(); };

        bool connect() {
            mg_mgr_init(&this->mgr);
            if (snprintf(this->_buffer, DEFAULT_BUFFER_SIZE, "ws://%s:%u",
                        this->ip.c_str(), this->port) < 0)
            {
                std::cerr << "ERROR: Failed to built the address string." << std::endl;
                return false;
            }
            this->c = mg_ws_connect(&this->mgr, this->_buffer, this->callback, &this->done, NULL);
            return true;
        }

        void loop(size_t timeout_ms) {
            while (c && !this->done) mg_mgr_poll(&mgr, timeout_ms);
        }

        void cleanup() {
            done = true;
            mg_mgr_free(&mgr);
        }
};

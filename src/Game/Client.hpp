#pragma once
#include "../mongoose.h"
#include "../Message/Message.hpp"
#include "../Shared/Player.hpp"
#include <atomic>
#include <iostream>
#include <string>
#include <vector>

#define DEFAULT_BUFFER_SIZE 1024

// NOTE: I think we need Message class or enum to send message from the main thread to the second thread(network)
//       to send some data if the user do X.
class Client {
    public:
        std::string ip;
        uint16_t port;
        void (*callback)(struct mg_connection *, int ev, void *ev_data);
        char _buffer[DEFAULT_BUFFER_SIZE];
        struct mg_mgr mgr;
        std::atomic<bool> done{false};
        struct mg_connection *c = nullptr;
        std::vector<std::string> _outbox;
        std::mutex _outbox_mtx;

        Player p;

        Client() = default;
        ~Client() { this->cleanup(); };

        bool connect(void *data) {
            this->p = {};
            mg_mgr_init(&this->mgr);
            if (snprintf(this->_buffer, DEFAULT_BUFFER_SIZE, "ws://%s:%u",
                        this->ip.c_str(), this->port) < 0)
            {
                std::cerr << "ERROR: Failed to built the address string." << std::endl;
                return false;
            }
            this->c = mg_ws_connect(&this->mgr, this->_buffer, this->callback, data, NULL);
            return true;
        }

        void loop(size_t timeout_ms) {
            while (c && !this->done) {
                mg_mgr_poll(&mgr, timeout_ms);

                std::lock_guard<std::mutex> lock(_outbox_mtx);
                if (!_outbox.empty() && c != nullptr) {
                    for (const auto& msg : _outbox) {
                        mg_ws_send(c, msg.c_str(), msg.size(), WEBSOCKET_OP_TEXT);
                    }
                    _outbox.clear();
                }
            }
        }

        // NOTE: Use this as disconnect mechanism too.
        void cleanup() {
            this->done = true;
            mg_mgr_free(&this->mgr);
        }

        void send(const struct Message &m) {
            char buf[DEFAULT_BUFFER_SIZE];
            int len = mg_snprintf(buf, sizeof(buf), "%M", print_msg, &m);
            std::lock_guard<std::mutex> lock(_outbox_mtx);
            _outbox.push_back(std::string(buf, len));
        }
};

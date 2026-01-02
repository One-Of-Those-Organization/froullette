#pragma once
#include "../mongoose.h"
#include "../Message/Message.hpp"
#include "../Shared/Player.hpp"
#include <atomic>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <cstdio>

#define DEFAULT_BUFFER_SIZE 1024

// Forward declaration
class Client;

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

// Global handle for the shims to access the active socket
// Static ensures internal linkage if included in multiple files (header-only style)
static EMSCRIPTEN_WEBSOCKET_T g_ws_handle = 0;

// Shims to redirect Mongoose calls to Emscripten WebSocket API
#undef mg_ws_send
#undef mg_ws_printf

// Shim for mg_ws_send
#define mg_ws_send(conn, data, len, op) \
    do { \
        if (g_ws_handle) { \
            if ((op) == WEBSOCKET_OP_TEXT) { \
                /* emscripten_websocket_send_utf8_text calculates length via strlen, ensure null-terminated if needed */ \
                /* But data might not be null terminated, so we might need a temp buffer if strictly text */ \
                /* However, for most text ops, we can just send. If 'data' is raw bytes, use binary */ \
                emscripten_websocket_send_utf8_text(g_ws_handle, (const char*)(data)); \
            } else { \
                emscripten_websocket_send_binary(g_ws_handle, (const char*)(data), (int)(len)); \
            } \
        } \
    } while (0)

// Shim for mg_ws_printf
#define mg_ws_printf(conn, op, fmt, ...) \
    do { \
        char _buf_[DEFAULT_BUFFER_SIZE]; \
        int _len_ = snprintf(_buf_, sizeof(_buf_), fmt, __VA_ARGS__); \
        if (_len_ > 0) { \
             if (g_ws_handle) { \
                if ((op) == WEBSOCKET_OP_TEXT) { \
                    _buf_[_len_] = '\0'; \
                    emscripten_websocket_send_utf8_text(g_ws_handle, _buf_); \
                } else { \
                    emscripten_websocket_send_binary(g_ws_handle, _buf_, _len_); \
                } \
             } \
        } \
    } while (0)

#endif

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

#ifdef __EMSCRIPTEN__
        // A dummy connection object to store fn_data and pass to the game handler
        struct mg_connection dummy_conn{};
#endif

        Client() = default;
        ~Client() { this->cleanup(); };

        bool connect(void *data) {
            this->p = {};
            if (snprintf(this->_buffer, DEFAULT_BUFFER_SIZE, "ws://%s:%u",
                        this->ip.c_str(), this->port) < 0)
            {
                std::cerr << "ERROR: Failed to built the address string." << std::endl;
                return false;
            }

#ifndef __EMSCRIPTEN__
            mg_mgr_init(&this->mgr);
            this->c = mg_ws_connect(&this->mgr, this->_buffer, this->callback, data, NULL);
            return this->c != nullptr;
#else
            if (!emscripten_websocket_is_supported()) {
                std::cerr << "ERROR: Emscripten WebSocket not supported." << std::endl;
                return false;
            }

            EmscriptenWebSocketCreateAttributes attr;
            emscripten_websocket_init_create_attributes(&attr);
            attr.url = this->_buffer;
            attr.protocols = NULL;
            attr.createOnMainThread = EM_TRUE;

            g_ws_handle = emscripten_websocket_new(&attr);
            if (g_ws_handle <= 0) {
                std::cerr << "ERROR: Failed to create WebSocket." << std::endl;
                return false;
            }

            // Setup dummy connection
            this->dummy_conn.fn_data = data;
            this->c = &this->dummy_conn;

            // Register callbacks
            emscripten_websocket_set_onopen_callback(g_ws_handle, this, Client::on_open_ems);
            emscripten_websocket_set_onmessage_callback(g_ws_handle, this, Client::on_message_ems);
            emscripten_websocket_set_onclose_callback(g_ws_handle, this, Client::on_close_ems);
            emscripten_websocket_set_onerror_callback(g_ws_handle, this, Client::on_error_ems);

            return true;
#endif
        }

        void loop(size_t timeout_ms) {
#ifndef __EMSCRIPTEN__
            while (c && !this->done) {
                mg_mgr_poll(&mgr, timeout_ms);

                std::lock_guard<std::mutex> lock(_outbox_mtx);
                if (!_outbox.empty() && c && !c->is_closing) {
                    for (const auto& msg : _outbox) {
                        mg_ws_send(c, msg.c_str(), msg.size(), WEBSOCKET_OP_TEXT);
                    }
                    _outbox.clear();
                }
            }
#else
            // Web is event-driven; loop logic is handled by browser event loop.
            (void)timeout_ms;
#endif
        }

        void cleanup() {
            this->done = true;
#ifndef __EMSCRIPTEN__
            mg_mgr_free(&this->mgr);
#else
            if (g_ws_handle) {
                emscripten_websocket_close(g_ws_handle, 1000, NULL);
                g_ws_handle = 0;
            }
#endif
        }

        // NOTE: Use this to send msg to server
        void send(const struct Message &m) {
            char buf[DEFAULT_BUFFER_SIZE];
            // Mongoose's mg_snprintf is useful, or use std::snprintf
            int len = mg_snprintf(buf, sizeof(buf), "%M", print_msg, &m);

#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(_outbox_mtx);
            _outbox.push_back(std::string(buf, len));
#else
            // On web, send immediately if connected
            if (g_ws_handle) {
                buf[len] = '\0'; // Ensure null termination for UTF8
                emscripten_websocket_send_utf8_text(g_ws_handle, buf);
            }
#endif
        }

#ifdef __EMSCRIPTEN__
    private:
        static EM_BOOL on_open_ems(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData) {
            Client* self = (Client*)userData;
            if (self && self->callback) {
                self->callback(self->c, MG_EV_WS_OPEN, NULL);
            }
            return EM_TRUE;
        }

        static EM_BOOL on_message_ems(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData) {
            Client* self = (Client*)userData;
            if (self && self->callback) {
                // Bridge Emscripten data to mg_ws_message struct
                struct mg_ws_message wm;
                wm.data.buf = (char*)e->data;
                wm.data.len = e->numBytes;
                wm.flags = 0; // Not used often in high-level handlers

                self->callback(self->c, MG_EV_WS_MSG, &wm);
            }
            return EM_TRUE;
        }

        static EM_BOOL on_close_ems(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData) {
            // Optional: Handle close event or auto-reconnect logic here
            return EM_TRUE;
        }

        static EM_BOOL on_error_ems(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData) {
            std::cerr << "WebSocket Error!" << std::endl;
            return EM_TRUE;
        }
#endif
};

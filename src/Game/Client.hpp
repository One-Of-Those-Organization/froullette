#pragma once
#include "../mongoose.h"
#include "../Message/Message.hpp"
#include "../Shared/Player.hpp"
#include <atomic>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <cstdio>

#define DEFAULT_BUFFER_SIZE 1024

// Forward declaration
class Client;

struct QueuedMessage {
    std::vector<uint8_t> data;
    int opcode; // WEBSOCKET_OP_TEXT or WEBSOCKET_OP_BINARY
};

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

static EMSCRIPTEN_WEBSOCKET_T g_ws_handle = 0;

#undef mg_ws_send
#undef mg_ws_printf

#define mg_ws_send(conn, data, len, op) \
    do { \
        if (g_ws_handle) { \
            if ((op) == WEBSOCKET_OP_TEXT) { \
                emscripten_websocket_send_utf8_text(g_ws_handle, (const char*)(data)); \
            } else { \
                emscripten_websocket_send_binary(g_ws_handle, (const char*)(data), (int)(len)); \
            } \
        } \
    } while (0)

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
        std::string url;
        void (*callback)(struct mg_connection *, int ev, void *ev_data);
        char _buffer[DEFAULT_BUFFER_SIZE];
        struct mg_mgr mgr;
        std::atomic<bool> done{false};
        std::atomic<bool> ws_connected{false};
        struct mg_connection *c = nullptr;
        std::vector<QueuedMessage> _outbox;
        std::mutex _outbox_mtx;

#ifdef __EMSCRIPTEN__
        struct mg_connection dummy_conn{};
#endif

        Client() = default;
        ~Client() { this->cleanup(); };

        bool connect(void *data) {
            if (snprintf(this->_buffer, DEFAULT_BUFFER_SIZE, "ws://%s", this->url.c_str()) < 0)
            {
                TraceLog(LOG_INFO, "NET: Failed to built the address string.");
                return false;
            }

#ifndef __EMSCRIPTEN__
            mg_mgr_init(&this->mgr);
            this->c = mg_ws_connect(&this->mgr, this->_buffer, this->callback, data, NULL);
            return this->c != nullptr;
#else
            if (!emscripten_websocket_is_supported()) {
                TraceLog(LOG_INFO, "NET: Emscripten WebSocket not supported.");
                return false;
            }

            EmscriptenWebSocketCreateAttributes attr;
            emscripten_websocket_init_create_attributes(&attr);
            attr.url = this->_buffer;
            attr.protocols = NULL;
            attr.createOnMainThread = EM_TRUE;

            g_ws_handle = emscripten_websocket_new(&attr);
            if (g_ws_handle <= 0) {
                TraceLog(LOG_INFO, "NET: Failed to create WebSocket.");
                return false;
            }

            this->dummy_conn.fn_data = data;
            this->c = &this->dummy_conn;

            emscripten_websocket_set_onopen_callback(g_ws_handle, this, Client::on_open_ems);
            emscripten_websocket_set_onmessage_callback(g_ws_handle, this, Client::on_message_ems);
            emscripten_websocket_set_onclose_callback(g_ws_handle, this, Client::on_close_ems);
            emscripten_websocket_set_onerror_callback(g_ws_handle, this, Client::on_error_ems);

            return true;
#endif
        }

        // Call this when MG_EV_WS_OPEN fires to flush queued messages
        void on_connected() {
            ws_connected = true;
            flush_outbox();
        }

        void loop(size_t timeout_ms) {
#ifndef __EMSCRIPTEN__
            while (!this->done) {
                if (!c) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                mg_mgr_poll(&mgr, timeout_ms);
                flush_outbox();
            }
#else
            (void)timeout_ms;
#endif
        }

        void cleanup() {
            this->done = true;
            this->ws_connected = false;
#ifndef __EMSCRIPTEN__
            mg_mgr_free(&this->mgr);
#else
            if (g_ws_handle) {
                emscripten_websocket_close(g_ws_handle, 1000, NULL);
                g_ws_handle = 0;
            }
#endif
        }

        void on_disconnected() {
            ws_connected = false;
            c = nullptr;
            std::lock_guard<std::mutex> lock(_outbox_mtx);
            _outbox.clear();
        }

        void send_binary(const void* data, size_t len) {
            queue_message(data, len, WEBSOCKET_OP_BINARY);
        }

        void send(const Message &msg) {
            uint8_t buf[MAX_MESSAGE_BIN_SIZE];
            size_t n = generate_network_field((Message*)&msg, buf);
            this->send_binary(buf, n);
        }

    private:
        void queue_message(const void* data, size_t len, int opcode) {
            {
                std::lock_guard<std::mutex> lock(_outbox_mtx);
                QueuedMessage qm;
                qm.opcode = opcode;
                const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
                qm.data.assign(p, p + len);
                if (opcode == WEBSOCKET_OP_TEXT) {
                    qm.data.push_back('\0');
                }
                _outbox.push_back(std::move(qm));
            }
            flush_outbox();
        }

        void flush_outbox() {
            if (!ws_connected) return;

            std::lock_guard<std::mutex> lock(_outbox_mtx);
            if (_outbox.empty()) return;

            for (const auto& msg : _outbox) {
                #ifndef __EMSCRIPTEN__
                if (c && !c->is_closing) {
                    mg_ws_send(c, msg.data.data(), msg.data.size(), msg.opcode);
                }
                #else
                if (g_ws_handle) {
                    if (msg.opcode == WEBSOCKET_OP_TEXT) {
                        emscripten_websocket_send_utf8_text(g_ws_handle, (const char*)msg.data.data());
                    } else {
                        emscripten_websocket_send_binary(g_ws_handle, (void*)msg.data.data(), (int)msg.data.size());
                    }
                }
                #endif
            }
            _outbox.clear();
        }

#ifdef __EMSCRIPTEN__
        static EM_BOOL on_open_ems(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData) {
            (void)e;
            (void)eventType;
            Client* self = (Client*)userData;
            if (self) {
                self->on_connected();
                if (self->callback) {
                    self->callback(self->c, MG_EV_WS_OPEN, NULL);
                }
            }
            return EM_TRUE;
        }

        static EM_BOOL on_message_ems(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData) {
            (void)eventType;
            Client* self = (Client*)userData;
            if (self && self->callback) {
                struct mg_ws_message wm;
                wm.data.buf = (char*)e->data;
                wm.data.len = e->numBytes;
                wm.flags = e->isText ? WEBSOCKET_OP_TEXT : WEBSOCKET_OP_BINARY;

                self->callback(self->c, MG_EV_WS_MSG, &wm);
            }
            return EM_TRUE;
        }

        static EM_BOOL on_close_ems(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData) {
            (void)eventType;
            (void)e;
            Client* self = (Client*)userData;
            if (self) {
                self->on_disconnected();
                if (self->callback) {
                    self->callback(self->c, MG_EV_CLOSE, NULL);
                }
            }
            return EM_TRUE;
        }

        static EM_BOOL on_error_ems(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData) {
            (void)eventType;
            (void)e;
            Client* self = (Client*)userData;
            TraceLog(LOG_INFO, "NET: Websocket error");
            if (self && self->callback) {
                self->callback(self->c, MG_EV_ERROR, (void*)"WebSocket error");
            }
            return EM_TRUE;
        }
#endif
};

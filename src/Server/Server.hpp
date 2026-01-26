#pragma once
#include "../Shared/Room.hpp"
#include "../mongoose.h"
#include <atomic>
#include <iostream>
#include <unordered_map>

#define STR_BUFFER_SIZE 64

#define UNUSED(x) (void)x

class Server {
public:
  size_t buffer_size = STR_BUFFER_SIZE;
  mg_mgr mgr;
  char *buffer = nullptr;
  char *secbuffer = nullptr;
  void (*callback)(mg_connection *c, int ev, void *ev_data);
  std::atomic<uint32_t> ccount = {1}; // to assign id
  Room rooms[MAX_ROOM_COUNT];
  std::unordered_map<int, Player> players;

  Server(const char *ip, uint16_t port,
         void (*callback)(mg_connection *c, int ev, void *ev_data)) {
    this->callback = callback;
    this->buffer = (char *)malloc(buffer_size);
    this->secbuffer = (char *)malloc(buffer_size);
    if (!this->buffer || ! this->secbuffer) {
      std::cerr << "ERROR: Failed to allocate buffer." << std::endl;
    }
    if (snprintf(buffer, buffer_size, "http://%s:%u", ip, port) < 0) {
      std::cerr << "ERROR: Failed to built the address string." << std::endl;
    }
    if (snprintf(secbuffer, buffer_size, "ws://%s:%u", ip, port + 1) < 0) {
      std::cerr << "ERROR: Failed to built the address string." << std::endl;
    }
    mg_mgr_init(&this->mgr);
    mg_http_listen(&this->mgr, buffer, callback, this);
    mg_http_listen(&this->mgr, secbuffer, callback, this);
  };

  void add_timer(size_t timeout_ms, int flag, void (*callback)(void *data),
                 void *data) {
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

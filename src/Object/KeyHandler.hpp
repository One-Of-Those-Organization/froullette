#pragma once

#include "Object.hpp"
#include "../Game/ArsEng.hpp"
#include <functional>
#include <raylib.h>

struct Handler {
    int id;
    int key;
    GameState state;
    std::function<void()> callback;
};

class KeyHandler: public Object {
    public:
        GameState *engine_state = nullptr;
        std::vector<Handler> handler;
        KeyHandler() {};
        virtual ~KeyHandler() = default;
        void render() override {};
        void logic(float dt) override {
            (void)dt;
            if (!this->engine_state) return;
            for (size_t i = 0; i < handler.size(); i++) {
                auto &current = handler[i];
                if (has_flag(*this->engine_state, current.state) && IsKeyReleased(current.key)) {
                    TraceLog(LOG_INFO, "this->state: %d and current->state: %d", this->state, current.state);
                    current.callback();
                }
            }
        };

        int add_new(int key, GameState state, std::function<void()> callback) {
            int id = handler.size();
            handler.push_back(Handler{id, key, state, callback});
            return id;
        }
};

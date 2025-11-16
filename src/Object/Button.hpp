#pragma once

#include "Object.hpp"
#include <string>

class Button : public Object {
    public:
        Vector2 *curpos = nullptr;
        std::string text;
        Color color[4];
        void (*callback)(void *data);

        Button() {}
        virtual ~Button() = default;

        void render() override {
            if (!show) return;
        }

        void logic(float dt) override {
            (void)dt;
        }
};

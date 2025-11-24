#pragma once
#include "Object.hpp"
#include <functional>

class Script: public Object {
    public:
        std::function<void()> callback;
        Script();
        ~Script() = default;

        void render() override {}
        void logic(float dt) override {
            (void)dt;
            if (this->callback) this->callback();
        }
};

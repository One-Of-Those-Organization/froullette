#pragma once

#include "Object.hpp"
#include <chrono>
#include <functional>

enum TimerType {
    LOOP, ONESHOT
};

class Timer: public Object {
    public:
        std::function<void()> callback;
        std::function<void()> miss_callback;
        std::chrono::time_point<std::chrono::steady_clock> start;
        TimerType tt;
        std::chrono::milliseconds target{0};
        bool _done = true;

        Timer() = default;
        explicit Timer(std::chrono::milliseconds target) : target(target) {}
        virtual ~Timer() = default;
        void start_timer() {
            this->start = std::chrono::steady_clock::now();
            this->_done = false;
        };

        void render() override {};
        void logic(float dt) override {
            (void)dt;
            if (this->_done) return;
            std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
            auto elapsed = now - start;
            if (elapsed >= target) {
                if (callback) callback();
                if (tt == TimerType::LOOP) {
                    start += target;
                } else {
                    _done = true;
                }
            } else {
                if (this->miss_callback) this->miss_callback();
            }
        };
};

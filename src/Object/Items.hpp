#pragma once

#include "Object.hpp"

class Items : public Object {
    const char *name;
    public:
        Items(const char *item_name): Object(), name(item_name) {};
        virtual ~Items() = default;
        void render() override {
            if (!this->show) return;
        };
        void logic(float dt) override {
            (void)dt;
        };
        virtual void callback() {};
};

#pragma once

#include "Object.hpp"
#include "../Shared/Player.hpp"
#include <functional>

class Items : public Object {
    std::function<void(Player * /* player */, void * /* additional_data */)> callback;
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
};

#pragma once

#include "Object.hpp"

enum class NeedleType: int32_t {
    NT_BLANK = 0,
    NT_LIVE = 1,
};

class Needle : public Object {
    public:
        NeedleType type;
        Needle(): Object() {};
        virtual ~Needle() = default;
        void render() override {};
        void logic(float dt) override {};
};

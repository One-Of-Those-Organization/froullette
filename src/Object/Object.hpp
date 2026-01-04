#pragma once

#include "../Game/GameState.hpp"
#include <raylib.h>

class Object {
    public:
        int id = -1;
        Rectangle rec;
        Texture2D *text;
        bool show;
        GameState state;
        void *engine;
        bool draw_in_canvas = true;
        Color color;

        Object() {
            this->engine = nullptr;
            this->text   = nullptr;
            this->show   = true;
            this->state  = GameState::ALL;
        };

        virtual ~Object() = default;
        virtual void render();
        virtual void logic(float dt);
};

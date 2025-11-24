#pragma once

#include "../Game/GameState.hpp"
#include "PositionInfo.hpp"
#include <raylib.h>

class Object {
    public:
        Rectangle rec;
        Texture2D *text;
        bool show;
        GameState state;
        void *engine;
        bool draw_in_canvas = true;
        Color color;
        PositionInfo position_info;
        bool is_resizable = false;

        Object() {
            this->engine = nullptr;
            this->text   = nullptr;
            this->show   = true;
            this->state  = GameState::ALL;
        };

        virtual ~Object() = default;
        virtual void render();
        virtual void logic(float dt);

        void update_position_from_relative(Vector2 new_window_size);
};

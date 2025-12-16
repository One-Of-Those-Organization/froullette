#pragma once

#include "../Game/GameState.hpp"
#include "PositionInfo.hpp"
#include <raylib.h>

class Object {
    public:
        Rectangle rec;
        Rectangle _saved_rec;
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

        void restore_rec() { this->rec = this->_saved_rec; }
        void store_rec() { this->_saved_rec = this->rec; }

        virtual ~Object() = default;
        virtual void render();
        virtual void logic(float dt);

        virtual void update_using_scale(float scale, Vector2 new_window_size);
};

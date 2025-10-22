#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"

static const Vector2 CANVAS_SIZE = Vector2{640, 480};

class ArsEng {
    public:

        RenderTexture2D canvas;
        ObjectManager om;
        GameState state;

        ArsEng(): om(), state(GameState::MENU) {
            canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
        }
        ~ArsEng() = default;
        void render_to_canvas() {
            BeginTextureMode(canvas);
            for (auto &obj: om.sorted) {
                if (!has_flag(state, obj->state)) continue;
                obj->render();
            }
            EndTextureMode();
        }
};

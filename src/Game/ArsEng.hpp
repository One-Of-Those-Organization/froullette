#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"

class ArsEng {
    public:

        RenderTexture2D canvas;
        ObjectManager om;
        GameState state;

        ArsEng(): om(), state(GameState::MENU) {
            canvas = LoadRenderTexture(1280, 720);
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

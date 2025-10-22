#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"

static const Vector2 CANVAS_SIZE = Vector2{640, 360};

class ArsEng {
    public:

        RenderTexture2D canvas;
        ObjectManager om;
        GameState state;

        ArsEng(): om(), state(GameState::MENU) {
            canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
            SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
            SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
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

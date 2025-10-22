#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"

static const Vector2 CANVAS_SIZE = Vector2{128, 72};

class ArsEng {
    public:

        RenderTexture2D canvas;
        ObjectManager om;
        GameState state;
        Vector2 canvas_size = {};

        ArsEng(): om(), state(GameState::MENU) {
            canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
            SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
            SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
            this->canvas_size.x = this->canvas.texture.width;
            this->canvas_size.y = this->canvas.texture.height;
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

        void update(float dt) {
            for (const auto &o: this->om.sorted) {
                o->logic(dt);
            }
        }
};

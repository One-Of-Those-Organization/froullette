#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"
#include "../Shader/ShadersManager.hpp"

static const Vector2 CANVAS_SIZE = Vector2{128, 72};

class ArsEng {
    public:

        RenderTexture2D canvas;
        ObjectManager om;
        ShadersManager sm;
        GameState state;
        Vector2 canvas_size = {};
        Vector2 cursor = {};
        Font font;

        ArsEng(): om(), state(GameState::MENU) {
            canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
            SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
            SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
            this->canvas_size.x = this->canvas.texture.width;
            this->canvas_size.y = this->canvas.texture.height;
            this->font =
                LoadFontEx("assets/Pixelify_Sans/PixelifySans-VariableFont_wght.ttf",
                           96, NULL, 95);
#ifdef MOBILE
            SetGestureEnabled(GESTURE_TAP);
#endif
        }

        ~ArsEng() {
            UnloadFont(this->font);
        };

        void check_and_recreate_canvas() {
            if (canvas.texture.id == 0) {
                UnloadRenderTexture(canvas);
                canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
                SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
                SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
                this->canvas_size.x = this->canvas.texture.width;
                this->canvas_size.y = this->canvas.texture.height;
            }
        }

        void render_to_canvas() {
            BeginTextureMode(canvas);
            ClearBackground(BLACK);
            for (auto &obj: om.sorted) {
                if (!has_flag(state, obj->state)) continue;
                obj->render();
            }
            EndTextureMode();
        }

        void update(float dt) {
#ifdef MOBILE
            this->cursor = GetTouchPosition();
#else
            this->cursor = GetMousePosition();
#endif
            for (const auto &o: this->om.sorted) {
                o->logic(dt);
            }
        }
};

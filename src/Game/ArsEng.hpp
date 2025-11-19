#pragma once

#include <raylib.h>
#include "../Object/ObjectManager.hpp"
#include "../Shader/ShadersManager.hpp"

static const Vector2 CANVAS_SIZE = Vector2{128, 72};

class ArsEng {
    public:
        Vector2 window_size;
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

            if (this->font.texture.id == 0)
                TraceLog(LOG_FATAL,
                         TextFormat("%s\n",
                         "Try to launch the game from the correct path."
                         " The game expect the `assets` folder in cwd."));

#ifdef MOBILE
            SetGesturesEnabled(GESTURE_TAP);
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
            Vector2 raw_cursor = GetTouchPosition();
#else
            Vector2 raw_cursor = GetMousePosition();
#endif
            this->cursor.x = (raw_cursor.x / this->window_size.x) * this->canvas_size.x;
            this->cursor.y = (raw_cursor.y / this->window_size.y) * this->canvas_size.y;
            for (const auto &o: this->om.sorted) {
                o->logic(dt);
            }
        }
};

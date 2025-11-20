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
        Vector2 canvas_cursor = {};
        Font font;
        std::vector<Object *> render_later;
        bool req_close;
        int active;
        int scale_factor[4] = {
            1, // smaller
            2, // 854x480
            3, // 1280x720
            4, // 1920x1080
        };

        void _set_active() {
            switch ((int)this->window_size.y) {
                case 480:  { active = 1; } break;
                case 720:  { active = 2; } break;
                case 1080: { active = 3; } break;
                default: { if (this->window_size.y < 480) active = 0; } break;
            }
        }

        ArsEng(Vector2 wsize): om(), state(GameState::MENU) {
            canvas = LoadRenderTexture(CANVAS_SIZE.x, CANVAS_SIZE.y);
            SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);
            SetTextureWrap(canvas.texture, TEXTURE_WRAP_CLAMP);
            this->window_size = wsize;
            this->req_close = false;
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
            this->_set_active();

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

        void render() {
            for (auto &obj: this->render_later) {
                if (!has_flag(state, obj->state)) continue;
                obj->render();
            }
            render_later.clear();
        }

        void render_to_canvas() {
            render_later.clear();
            BeginTextureMode(canvas);
            ClearBackground(BLACK);
            for (auto &obj: om.sorted) {
                if (!has_flag(state, obj->state)) continue;
                if (obj->draw_in_canvas) obj->render();
                else render_later.push_back(obj);
            }
            EndTextureMode();
        }

        void update(float dt) {
#ifdef MOBILE
            this->cursor = GetTouchPosition();
#else
            this->cursor = GetMousePosition();
#endif
            this->canvas_cursor.x =
                (this->cursor.x / this->window_size.x) * this->canvas_size.x;
            this->canvas_cursor.y =
                (this->cursor.y / this->window_size.y) * this->canvas_size.y;
            for (const auto &o: this->om.sorted) {
                if (!has_flag(state, o->state)) continue;
                if (o->show) o->logic(dt);
            }
        }

        int calcf(int value) {
            return value * this->scale_factor[active];
        }
};

#pragma once

#include <cstdint>
#include <raylib.h>
#include "../Game/ArsEng.hpp"
#include "../Game/Game.hpp"

class Window {
public:
    Vector2 size;
    Vector2 oldsize;
    uint32_t fps;
    const char *name;
    ArsEng *engine;

    Window(Vector2 size, uint32_t fps, const char *name)
        : size(size), oldsize(size), fps(fps), name(name) {
        InitWindow(size.x, size.y, name);
        SetTargetFPS(fps);

        engine = new ArsEng(size);
        gameInit(engine);
        SetExitKey(KEY_NULL);
    };

    ~Window() {
        gameDeinit(this->engine);
        CloseWindow();
        delete engine;
    };

    bool loop() {
        while (!WindowShouldClose()) {
            if (engine->req_close) break;
            if (engine->_req.t != DONE) {
                switch (engine->_req.t) {
                case TFULLSCREEN: { this->fullscreen_window(); } break;
                case RESIZE: { this->resize_window(engine->_req.data.v); } break;
                default:
                    break;
                }
            }
            float dt = GetFrameTime();
            engine->check_and_recreate_canvas();
            engine->update(dt);
            engine->render_to_canvas();
            BeginDrawing();

            // Do post-processing here
            Texture2D *txt = &engine->canvas.texture;
            DrawTexturePro(*txt,
                           Rectangle{0, 0, (float)txt->width, -(float)txt->height},
                           Rectangle{0, 0, size.x, size.y},
                           Vector2{0, 0}, 0.0f, WHITE);

            // Render later object (for ui and stuff) to the big canvas
            engine->render();

            // Render the big canvas
            Texture2D *bigtxt = &engine->bigcanvas.texture;
            DrawTexturePro(*bigtxt, Rectangle{0, 0, (float)bigtxt->width, -(float)bigtxt->height},
                           Rectangle{0, 0, size.x, size.y},
            Vector2{0, 0}, 0.0f, WHITE);

            EndDrawing();
        }
        return true;
    }

    void fullscreen_window() {
        if (IsWindowFullscreen()) {
            this->resize_window(this->oldsize);
        } else {
            this->resize_window(Vector2(GetMonitorWidth(0) , GetMonitorHeight(0)));
        }
        ToggleFullscreen();
        engine->_req.t = DONE;
    }

    void resize_window(Vector2 newsize) {
        this->oldsize = this->size;
        SetWindowSize(newsize.x, newsize.y);
        engine->window_size = newsize;
        this->size = newsize;
        engine->_req.t = DONE;
    }
};

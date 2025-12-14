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
    };

    ~Window() {
        gameDeinit(this->engine);
        CloseWindow();
        delete engine;
    };

    bool loop() {
        if (this->engine) this->engine->window_ptr = (void *)this;
        while (!WindowShouldClose()) {
            if (engine->req_close) break;
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

            // Render later object (for ui and stuff)
            engine->render();

            EndDrawing();
        }
        return true;
    }
    void resize_window(Vector2 newsize) {
        this->oldsize = this->size;
        SetWindowSize(newsize.x, newsize.y);
        this->size = newsize;
        if (this->engine) this->engine->handle_window_resize(newsize);
    }
};

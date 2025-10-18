#pragma once

#include <cstdint>
#include <raylib.h>
#include "../Game/ArsEng.hpp"
#include "../Game/Game.hpp"

class Window {
    public:
        Vector2 size;
        uint32_t fps;
        const char *name;
        ArsEng *engine;
    Window(Vector2 size, uint32_t fps, const char *name)
        : size(size), fps(fps), name(name) {
            InitWindow(size.x, size.y, name);
            SetTargetFPS(fps);

            engine = new ArsEng();
            gameInit(engine);
        };
    ~Window() {
        CloseWindow();
        delete engine;
    };
    bool loop() {
        while (!WindowShouldClose()) {
            engine->render_to_canvas();
            BeginDrawing();

            // Do post-processing here
            Texture2D *txt = &engine->canvas.texture;
            DrawTexturePro(*txt,
                    Rectangle{0, 0, (float)txt->width, -(float)txt->height},
                    Rectangle{0, 0, size.x, size.y},
                    Vector2{0, 0}, 0.0f, WHITE);

            EndDrawing();
        }
        return true;
    }
};

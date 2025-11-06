#pragma once
#include "ArsEng.hpp"
#include "../Object/Balls.hpp"
#include "../Object/Desk.hpp"
#include "../Shader/Trapezoid_shader.hpp"

static void gameInit(ArsEng *engine) {
    int z = 1;
    Vector2 wsize = {
        engine->canvas_size.x,
        engine->canvas_size.y,
    };

    // Load Object
    // auto ball = new Balls();
    // ball->rec = {10, 10, 10, 10};
    // ball->engine = engine;
    // ball->speed = {50, 50};
    // engine->om.add_object(ball, z++);

    auto desk = new Desk();
    desk->angle = {0.0f, 0.4f};
    desk->rec = {10, wsize.y / 2, wsize.x - 20, wsize.y};
    engine->om.add_object(desk, z++);

}

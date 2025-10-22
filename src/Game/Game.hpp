#pragma once
#include "ArsEng.hpp"
#include "../Object/Balls.hpp"

static void gameInit(ArsEng *engine) {
    int z = 1;
    auto rec = new Balls();
    rec->rec = {5, 5, 5, 5};
    rec->engine = engine;
    rec->speed = {100, 100};
    engine->om.add_object(rec, z++);
}

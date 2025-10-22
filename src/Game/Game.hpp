#pragma once
#include "ArsEng.hpp"

static void gameInit(ArsEng *engine) {
    int z = 1;
    auto rec = new Object();
    rec->rec = {20, 20, 20, 20};
    rec->engine = engine;
    rec->speed = {100, 0};
    engine->om.add_object(rec, z++);
}

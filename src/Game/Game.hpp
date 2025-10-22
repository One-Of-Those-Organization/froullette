#pragma once
#include "ArsEng.hpp"

static void gameInit(ArsEng *engine) {
    int z = 1;
    auto rec = new Object();
    rec->rec = {0, 20, 20, 20};
    engine->om.add_object(rec, z++);
}

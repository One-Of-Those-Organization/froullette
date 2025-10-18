#pragma once
#include "ArsEng.hpp"

static void gameInit(ArsEng *engine) {
    auto rec = new Object();
    rec->rec = {0, 0, 100, 100};
    engine->om.add_object(rec, 1);
    (void)engine;
}

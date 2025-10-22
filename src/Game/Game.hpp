#pragma once
#include "ArsEng.hpp"

static void gameInit(ArsEng *engine) {
    auto rec = new Object();
    rec->rec = {50, 50, 25, 25};
    engine->om.add_object(rec, 1);
    (void)engine;
}

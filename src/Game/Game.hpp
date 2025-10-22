#pragma once
#include "ArsEng.hpp"

static void gameInit(ArsEng *engine) {
    auto rec = new Object();
    rec->rec = {0, 20, 20, 20};
    engine->om.add_object(rec, 1);
}

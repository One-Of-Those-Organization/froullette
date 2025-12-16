#pragma once
#include <raylib.h>

struct PositionInfo {
    bool center_x  = false;
    bool center_y  = false;
    bool use_range = false;
    Vector2 range  = {0, 0}; // NOTE: Not used for now but the value should be: 0..1
    Vector2 offset = {0, 0};
};

#pragma once
#include <raylib.h>

struct PositionInfo {
    float relative_x = 0.0f;      // 0.0 to 1.0 (percentage of screen width)
    float relative_y = 0.0f;      // 0.0 to 1.0 (percentage of screen height)
    bool center_x = false;        // Should be centered horizontally
    bool center_y = false;        // Should be centered vertically
    Vector2 offset = {0, 0};      // Additional offset after relative positioning
    bool use_relative = false;    // Whether to use relative positioning
};

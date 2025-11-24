#include "Object.hpp"

void Object::render() {
    if (!show) return;

    if (text != nullptr && text->width > 0 && text->height > 0) {
        DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                rec, Vector2(0, 0), 0.0f, this->color);
    } else {
        DrawRectangleRec(rec, this->color);
    }
}
void Object::logic(float dt) {
    (void)dt;
};
void Object::update_position_from_relative(Vector2 new_window_size) {
    if (!is_resizable || !position_info.use_relative) return;

    if (position_info.center_x) {
        rec.x = (new_window_size.x - rec.width) / 2.0f + position_info.offset.x;
    } else {
        rec.x = new_window_size.x * position_info.relative_x + position_info.offset.x;
    }

    if (position_info.center_y) {
        rec.y = (new_window_size.y - rec.height) / 2.0f + position_info.offset.y;
    } else {
        rec.y = new_window_size.y * position_info.relative_y + position_info.offset.y;
    }
}

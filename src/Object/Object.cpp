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

void Object::update_using_scale(float scale, Vector2 new_window_size) {
    if (!is_resizable) return;

    if (position_info.center_x) {
        rec.x = (new_window_size.x - rec.width) / 2.0f + position_info.offset.x;
    } else {
        this->restore_rec();
        rec.x += position_info.offset.x;
    }

    if (position_info.center_y) {
        rec.y = ((new_window_size.y - rec.height) / 2.0f) + position_info.offset.y;
    } else {
        this->restore_rec();
        rec.y += position_info.offset.y;
    }
}

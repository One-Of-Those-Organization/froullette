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

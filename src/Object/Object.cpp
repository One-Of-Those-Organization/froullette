#include "Object.hpp"
#include "../Game/ArsEng.hpp"

void Object::render() {
    if (!show) return;

    if (text != nullptr && text->width > 0 && text->height > 0) {
        DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                rec, Vector2(0, 0), 0.0f, WHITE);
    } else {
        // DrawRectangleRec(rec, WHITE);
        DrawCircleV(Vector2{rec.x, rec.y}, rec.width, WHITE);
    }
}
void Object::logic(float dt) {
    if (engine) {
        ArsEng *e = (ArsEng*) this->engine;
        if (e->canvas_size.x <= this->rec.x + this->rec.width) {
            this->speed.x *= -1;
        }

        if (this->rec.x - this->rec.width <= 0) {
            this->speed.x *= -1;
        }
        this->rec.x += this->speed.x * dt;
    }
};

#pragma once

#include "Object.hpp"
#include "../Game/ArsEng.hpp"

class Balls: public Object {
    public:
        Vector2 speed;
        Balls() {};
        virtual ~Balls() = default;
        void render() override {
            if (!show) return;

            if (text != nullptr && text->width > 0 && text->height > 0) {
                DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                        rec, Vector2(0, 0), 0.0f, GRAY);
            } else {
                DrawCircleV(Vector2{rec.x, rec.y}, rec.width, GRAY);
            }
        }

        void logic(float dt) override {
            if (engine) {
                ArsEng *e = (ArsEng*) this->engine;
                if (e->canvas_size.x <= this->rec.x + this->rec.width ||
                        this->rec.x - this->rec.width <= 0) {
                    this->rec.x = e->canvas_size.x <= this->rec.x + this->rec.width
                        ? e->canvas_size.x - this->rec.width
                        : 0 + this->rec.width;
                    this->speed.x *= -1;
                }

                this->rec.x += this->speed.x * dt;

                if (e->canvas_size.y <= this->rec.y + this->rec.height ||
                        this->rec.y - this->rec.height <= 0) {
                    this->rec.y = e->canvas_size.y <= this->rec.y + this->rec.width
                        ? e->canvas_size.y - this->rec.width
                        : 0 + this->rec.width;
                    this->speed.y *= -1;
                }

                this->rec.y += this->speed.y * dt;
            }
        }
};

#ifndef OBJECT_H_
#define OBJECT_H_

#include "../Game/GameState.hpp"
#include <raylib.h>

struct Game;

class Object {
    public:
        Rectangle rec;
        Texture2D *text;
        bool show;
        GameState state;
        Game *game;

        Object() {};
        virtual ~Object() = default;
        virtual void render() {
            if (!show) return;

            if (text != nullptr && text->width > 0 && text->height > 0) {
                DrawTexturePro(*text, Rectangle(0, 0, text->width, text->height),
                        rec, Vector2(0, 0), 0.0f, WHITE);
            } else {
                // DrawRectangleRec(rec, WHITE);
                DrawCircleV(Vector2{rec.x, rec.y}, rec.width, WHITE);
            }
        };
        virtual void logic(float dt) {
            (void)dt;
        };
};

#endif // OBJECT_H_

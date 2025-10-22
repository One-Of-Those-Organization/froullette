#ifndef OBJECT_H_
#define OBJECT_H_

#include "../Game/GameState.hpp"
#include <raylib.h>

class Object {
    public:
        Rectangle rec;
        Texture2D *text;
        bool show;
        GameState state;
        void *engine;

        Object() {};
        virtual ~Object() = default;
        virtual void render();
        virtual void logic(float dt);
};

#endif // OBJECT_H_

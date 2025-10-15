#ifndef OBJTURNINDICATOR_H_
#define OBJTURNINDICATOR_H_

#include "../Game/Game.hpp"
#include "../Game/GameTurn.hpp"
#include "Object.hpp"

struct ObjTurnIndicator : public Object {
  public:
    ObjTurnIndicator(Rectangle rec, int z_index, const char *name)
        : Object(rec, z_index, name) {}

    virtual void render() {
        if (!mShow)
            return;

        DrawRectangleRec(mRec,
                         mGame_ptr->mTurn == GameTurn::PLAYER1 ? BLACK : WHITE);
    }

    virtual void logic(float dt) { (void)dt; }

    virtual ~ObjTurnIndicator() {}
};

#endif // OBJTURNINDICATOR_H_

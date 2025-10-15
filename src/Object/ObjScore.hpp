#ifndef OBJSCORE_H_
#define OBJSCORE_H_

#include "ObjText.hpp"

struct ObjScore : public ObjText {
public:
    GameTurn mBind;

    ObjScore(Rectangle rec, int z_index, const char *name, const char *text, Color color, int size, GameTurn bind)
        : ObjText(rec, z_index, name, text, color, size) {
        mBind = bind;
    }

    virtual void logic(float dt) {
        (void)dt;
        this->update_label();
    }
    virtual ~ObjScore() {}

    void update_label() {
        mText = TextFormat("%d", mGame_ptr->mScore[(int)mBind]);
    }
};

#endif // OBJSCORE_H_

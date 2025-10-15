#ifndef OBJBTNSKIP_H_
#define OBJBTNSKIP_H_

#include "../Game/Game.hpp"
#include "ObjButton.hpp"

struct ObjBtnSkip : public ObjButton {
    ObjBtnSkip(Rectangle rec, int z_index, const char *name, const char *text,
               Color color, Color bg_color, int size, int padding,
               std::function<void()> onClick)
        : ObjButton(rec, z_index, name, text, color, bg_color, size, padding,
                    onClick) {}

    virtual void logic(float dt) override {
        (void)dt;

        if (mGame_ptr->mWindow_ptr == nullptr)
            return;

        if (mGame_ptr->mDice <= 0 && !mGame_ptr->mVSBot)
            mShow = true;
        else if ((mGame_ptr->mDice <= 0 && mGame_ptr->mVSBot &&
                 mGame_ptr->mTurn != GameTurn::PLAYER2) && mGame_ptr->mBotCanMove)
            mShow = true;
        else
            mShow = false;

        // The button logic stuff
        if (!mShow)
            return;

        Rectangle calculated_rec =
            Rectangle(mRec.x - mPad, mRec.y - mPad, mRec.width + (mPad * 2),
                      mRec.height + (mPad * 2));
        if (CheckCollisionPointRec(mGame_ptr->mCursorPos, calculated_rec)) {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                mOnClick();
            mHovered = true;
        } else {
            mHovered = false;
        }
    }
    virtual ~ObjBtnSkip() {}
};

#endif // OBJBTNSKIP_H_

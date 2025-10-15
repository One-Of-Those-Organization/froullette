#ifndef OBJ_BEAD_H_
#define OBJ_BEAD_H_

#include "../Game/Game.hpp"
#include "../Game/GameTurn.hpp"
#include "../ManagedSound/ManagedSound.hpp"
#include "Object.hpp"

#include <cassert>
#include <cstring>
#include <functional>

#define PADDING 10

struct ObjBead : public Object {
  private:
    Vector2 _xy_gen_helper() {
        Vector2 res = {mBoard->mRec.x, mBoard->mRec.y};
        if (mPos > 0 && mPos <= 4) {
            res.x += (20 * mGame_ptr->mScale) * (3 + mPos);
            if (mGroup == GameTurn::PLAYER1)
                res.y += (20 * mGame_ptr->mScale) * 2;
        } else if (mPos >= 5 && mPos <= 12) {
            res.x += (20 * mGame_ptr->mScale) * (12 - mPos);
            res.y += 20 * mGame_ptr->mScale;
        } else {
            res.x += (20 * mGame_ptr->mScale) * (mPos - 13);
            if (mGroup == GameTurn::PLAYER1)
                res.y += (20 * mGame_ptr->mScale) * 2;
        }
        return res;
    }
    sptr_t<Object> mBoard;
    int mIndex;
    Color mUtilsColor;
    Color mUtilsColorInvert;

  public:
    int mPos;
    bool mOut;
    GameTurn mGroup;
    ManagedSound *mSound;
    Shader *mShader;

    std::function<void()> mOnClick = nullptr;

    ObjBead(Rectangle rec, int z_index, const char *name, sptr_t<Object> board,
            GameTurn group, Color utils_color, Color utils_color_invert)
        : Object(rec, z_index, name) {
        mPos = 0;
        mOut = false;
        mBoard = board;
        mGroup = group;
        size_t jump_for = strlen("bead_p1_");
        assert(jump_for < strlen(name));
        mIndex = atoi(name + jump_for) + 1;
        mUtilsColor = utils_color;
        mUtilsColorInvert = utils_color_invert;
        mShader = nullptr;
    }

    virtual void render() {
        if (!mShow || !mOut)
            return;

        Vector2 new_pos = _xy_gen_helper();
        mRec.x = new_pos.x;
        mRec.y = new_pos.y;

        mRec.width = 20 * mGame_ptr->mScale;
        mRec.height = 20 * mGame_ptr->mScale;

        if (mText != nullptr && mText->width > 0 && mText->height > 0) {

            int rec_size = 9 * mGame_ptr->mScale;
            if (mGame_ptr->mTurn == mGroup) {
                BeginShaderMode(*mShader);

                DrawTexturePro(*mText, Rectangle(0, 0, mText->width, mText->height),
                        Rectangle{mRec.x + PADDING, mRec.y + PADDING, mRec.width - PADDING * 2, mRec.height - PADDING * 2},
                        Vector2(0, 0), 0.0f, WHITE);

                EndShaderMode();

                // Disable index rendering when vsbot and bot turn.
                if (mGame_ptr->mTurn == GameTurn::PLAYER2 && mGame_ptr->mVSBot)
                    return;
                DrawRectangleRec(
                    Rectangle(mRec.x, mRec.y, rec_size / 2.0f, rec_size),
                    mUtilsColorInvert);
                DrawTextPro(mGame_ptr->mFont, TextFormat("%d", mIndex),
                            Vector2(mRec.x, mRec.y), Vector2(0, 0), 0.0f,
                            rec_size, 10, mUtilsColor);
            } else {
                DrawTexturePro(*mText, Rectangle(0, 0, mText->width, mText->height),
                        Rectangle{mRec.x + PADDING, mRec.y + PADDING, mRec.width - PADDING * 2, mRec.height - PADDING * 2},
                        Vector2(0, 0), 0.0f, WHITE);
            }
        } else {
            DrawRectangleRec(mRec, RED);
        }
    }

    virtual void logic(float dt) {
        (void)dt;
        // Disable logic when no window and vsbot.
        if (mGame_ptr->mWindow_ptr == nullptr ||
            (mGame_ptr->mVSBot && mGame_ptr->mTurn == GameTurn::PLAYER2))
            return;
        if (CheckCollisionPointRec(mGame_ptr->mCursorPos, mRec)) {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                mOnClick();
            }
        }
    }

    virtual ~ObjBead() {}
};

#endif // OBJ_BEAD_H_

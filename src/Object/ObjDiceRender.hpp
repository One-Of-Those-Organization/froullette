#ifndef OBJ_DICERENDER_H_
#define OBJ_DICERENDER_H_

#include "../Game/Game.hpp"
#include "../Window/Window.hpp"
#include "Object.hpp"

struct ObjDiceRender : public Object {
  public:
    uint8_t mDice;

    ObjDiceRender(Rectangle rec, int z_index, const char *name)
        : Object(rec, z_index, name) {
        mDice = 0;
    }

    virtual void render() {
        if (!mShow)
            return;

        mDice = mGame_ptr->mDice;
        if (mText != nullptr && mText->width > 0 && mText->height > 0) {
            Vector2 *wsize = mGame_ptr->mWindow_ptr->get_window_size();
            mRec = {.x = wsize->x / 2.0f,
                    .y = wsize->y - mGame_ptr->mScale * 25.0f,
                    .width = (float)mText->width * (mGame_ptr->mScale - 2),
                    .height = (float)mText->height * (mGame_ptr->mScale - 2)};

            // If got 0 render text.
            if (mDice == 0) {
                const char *text_content = "Skip turn. [Space]";
                const int text_size = 12 * mGame_ptr->mScale;
                const int text_spacing = mGame_ptr->mScale;
                int text_len = MeasureTextEx(mGame_ptr->mFont, text_content,
                                             text_size, text_spacing)
                                   .x;
                DrawTextPro(mGame_ptr->mFont, text_content,
                            Vector2(mRec.x - (text_len / 2.0f), mRec.y), Vector2(0, 0), 0.0f,
                            text_size, text_spacing, WHITE);
                return;
            }

            mRec.x -= (mDice * mGame_ptr->mScale * 7 + mRec.width) / 2.0f;

            for (int i = 0; i < mDice; i++) {
                Rectangle transform_rec(mRec.x + (i * mGame_ptr->mScale * 7),
                                        mRec.y, mRec.width, mRec.height);
                DrawTexturePro(*mText,
                               Rectangle(0, 0, mText->width, mText->height),
                               transform_rec, Vector2(0, 0), 0.0f, WHITE);
            }

        } else {
            DrawRectangleRec(mRec, RED);
        }
    }
    virtual ~ObjDiceRender() {}
};

#endif // OBJ_DICERENDER_H_

#ifndef OBJSLIDE_H_
#define OBJSLIDE_H_

#include "../Game/Game.hpp"
#include "Object.hpp"

struct ObjSlide : public Object {
  public:
    std::vector<Texture2D *> mSlide;
    int mIndex;

    ObjSlide(Rectangle rec, int z_index, const char *name)
        : Object(rec, z_index, name) {
        mIndex = 0;
    }
    ~ObjSlide() = default;

    void add_slide(Texture2D *text) { mSlide.push_back(text); }

    bool _next_index() {
        if (mIndex + 1 >= (int)mSlide.size()) {
            return false;
        }
        mIndex++;
        return true;
    }

    // void toggle_other_object() {
    //     if (!mGame_ptr) return;
    //     for (auto& item: mGame_ptr->mObjMan.mData) {
    //         if (item->mTag == GameState::PLAYMENU && item->mName != "slide") {
    //             item->mShow = !item->mShow;
    //         }
    //     }
    // }

    void logic(float dt) {
        (void)dt;
        if (mGame_ptr->mWindow_ptr == nullptr || !mShow)
            return;

        if (mText != mSlide[mIndex])
            mText = mSlide[mIndex];

        if (IsKeyReleased(KEY_SPACE)) {
            if (!_next_index()){
                mIndex = 0;
            }
        }

        if (CheckCollisionPointRec(mGame_ptr->mCursorPos, mRec)) {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (!_next_index()){
                    mIndex = 0;
                }
            }
        }
    }
};

#endif // OBJSLIDE_H_

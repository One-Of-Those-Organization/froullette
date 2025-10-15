#ifndef OBJ_BUTTON_H_
#define OBJ_BUTTON_H_

// TODO: Make padding autoapply so it didnt need to be edited on rendering only
#include "../Game/Game.hpp"
#include "Object.hpp"
#include <functional>

struct ObjButton : public Object {
  public:
    std::string mText;
    Color mColor;
    Color mBgColor;
    Color mHoverColor;
    Color mHoverBgColor;
    int mPad;
    int mSize;
    int mSpacing;
    bool mHovered;
    std::function<void()> mOnClick = nullptr;

    ObjButton(Rectangle rec, int z_index, const char *name, const char *text,
              Color color, Color bg_color, int size, int padding,
              std::function<void()> onClick)
        : Object(rec, z_index, name), mOnClick(onClick) {
        mText = text;
        mColor = color;
        mBgColor = bg_color;
        mHoverColor = bg_color;
        mHoverBgColor = color;
        mSize = size;
        mPad = padding;
        mHovered = false;
        // RIPPED from raylib straight up.
        int defaultFontSize = 10;
        if (mSize < defaultFontSize)
            mSize = defaultFontSize;
        mSpacing = mSize / defaultFontSize;
    }

    int get_width() {
        if (mGame_ptr != nullptr) {
            Vector2 size =
                MeasureTextEx(mGame_ptr->mFont, mText.c_str(), mSize, mSpacing);
            return size.x;
        }
        return -1;
    }

    virtual void render() override {
        if (!mShow)
            return;

        Color &fgcolor = mHovered ? mBgColor : mColor;
        Color &bgcolor = mHovered ? mColor : mBgColor;
        if (mRec.width > 0 && mRec.height > 0)
            DrawRectangleRec(Rectangle(mRec.x - mPad, mRec.y - mPad,
                                       mRec.width + (mPad * 2),
                                       mRec.height + (mPad * 2)),
                             bgcolor);

        if (mText != "") {
            DrawTextPro(mGame_ptr->mFont, mText.c_str(),
                        Vector2(mRec.x, mRec.y), Vector2(0, 0), 0.0f, mSize,
                        mSpacing, fgcolor);
        }
    }

    // TODO: make calculated_rec apply mRec
    virtual void logic(float dt) override {
        (void)dt;
        if (mGame_ptr->mWindow_ptr == nullptr || !mShow)
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
    };

    virtual ~ObjButton() {};
};

#endif // OBJ_BUTTON_H_

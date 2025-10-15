#include "Object.hpp"

Object::Object(Rectangle rec, int z_index, const char *name) {
    mRec = rec;
    mZIndex = z_index;
    mName = name;
    mText = nullptr;
    mShow = true;
    mTag = GameState::ALL;
}

Object::Object(Rectangle rec, int z_index, const char *name, Texture2D *text) {
    mRec = rec;
    mZIndex = z_index;
    mName = name;
    mText = text;
    mShow = true;
    mTag = GameState::ALL;
}

void Object::render() {
    if (!mShow)
        return;

    if (mText != nullptr && mText->width > 0 && mText->height > 0) {
        DrawTexturePro(*mText, Rectangle(0, 0, mText->width, mText->height),
                       mRec, Vector2(0, 0), 0.0f, WHITE);
    } else {
        DrawRectangleRec(mRec, RED);
    }
}

void Object::logic(float dt) { (void)dt; }

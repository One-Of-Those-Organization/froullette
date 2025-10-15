#ifndef GAME_H_
#define GAME_H_

#include "../ObjectManager/ObjectManager.hpp"
#include "../Shaders/ShadersManager.hpp"
#include "../TextureManager/TextureManager.hpp"
#include "../SoundManager/SoundManager.hpp"
#include "GameState.hpp"
#include "GameTurn.hpp"
#include <raylib.h>

#define VOL_NORMAL 0.2f
#define VOL_LOW 0.1f

struct Window;

struct Game {
  public:
    TextureManager mTexMan;
    ObjectManager mObjMan;
    ShadersManager mSMan;
    SoundManager mSouMan;
    Window *mWindow_ptr;
    Font mFont;
    Music mMusic;

    GameState mStateOrTag;
    GameState mOldStateOrTag;
    int mScale;

    Vector2 mCursorPos;
    bool mWantExit;

    // For the ingame
    GameTurn mTurn;
    Game();
    ~Game();

    void init(Window *w);
    void handle_logic(float dt);
    void handle_drawing(float dt);
    void handle_key(float dt);
    void change_state(GameState new_state);
    void revert_state();
    void exit_game();

    void _sync_scale();
};

void set_winsound_playable(bool p);

#endif // GAME_H_

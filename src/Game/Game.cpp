#include "Game.hpp"
#include "../Window/Window.hpp"
#include "GameBot.hpp"
#include "GameUtils.hpp"

/* #define DEBUG_MODE */

Game::Game() {
    mTexMan = TextureManager();
    mObjMan = ObjectManager();
    mSMan = ShadersManager();
    mSouMan = SoundManager();
    mWindow_ptr = nullptr;
    mStateOrTag = GameState::MENU;
    mOldStateOrTag = mStateOrTag;
    mScale = 0;
    mFont = Font();
    mCursorPos = Vector2();
    mWantExit = false;

    mTurn = GameTurn::PLAYER1;
}

Game::~Game() {
    UnloadFont(mFont);
    UnloadMusicStream(mMusic);
}

void Game::init(Window *w) {
    mWindow_ptr = w;
    mObjMan.mGame_ptr = this;
    int z_index = 1;

    mWindow_ptr->set_app_icon("./assets/nayeon.png");
    mSouMan.load_sound("./assets/bead-placed.wav", "bead_placed");
    mSouMan.load_sound("./assets/victory-96688.mp3", "victory");
    mMusic = LoadMusicStream("assets/background-music.mp3");

    mFont =
        LoadFontEx("assets/Pixelify_Sans/PixelifySans-VariableFont_wght.ttf",
                   96, NULL, 95);
    if (mFont.texture.id == 0)
        TraceLog(LOG_FATAL,
                 TextFormat("%s\n",
                            "Try to launch the game from the correct path."
                            " The game expect the `assets` folder in cwd."));

    // mSMan.add_shader_from_mem("menu", nullptr, menu_shaders);
    // mSMan.add_shader_from_mem("ingame", nullptr, ingame_shaders);
    // mSMan.add_shader_from_mem("bead", nullptr, bead_shaders);

    _sync_scale();

    // Play the music.
    PlayMusicStream(mMusic);
    SetMusicVolume(mMusic, VOL_NORMAL);
}

void Game::handle_logic(float dt) {
    // To keep the stream alive it seems.
    UpdateMusicStream(mMusic);
    GameState current_state = mStateOrTag;

    // If the current state not menu then decrase the volume more.
    if (current_state == GameState::INGAME ||
        current_state == GameState::FINISHED)
        SetMusicVolume(mMusic, VOL_LOW);

    for (auto &d : mObjMan.mData) {
        if (current_state != mStateOrTag) {
            // TODO: Do something here...
            break;
        }

        if (has_flag(d->mTag, mStateOrTag))
            d->logic(dt);
    }
}

void Game::handle_drawing(float dt) {
    (void)dt;

    for (auto &d : mObjMan.mData) {
        if (has_flag(d->mTag, mStateOrTag))
            d->render();
    }
}

void Game::handle_key(float dt) {
    (void)dt;

    switch (mStateOrTag) {
    case GameState::ALL: {
        break;
    }
    case GameState::INGAME: {

        if (IsKeyReleased(KEY_ESCAPE)) {
            change_state(GameState::MENU);
            SetMusicVolume(mMusic, VOL_NORMAL);
        }
    }

#ifdef DEBUG_MODE
#endif

    case GameState::PLAYMENU: {
        break;
    }
    case GameState::MENU: {
        break;
    }
    case GameState::SETTINGS: {
        break;
    }
    case GameState::FINISHED: {
        break;
    }
    case GameState::TUTORIAL: {
        break;
    }
    default:
        break;
    }
}

void Game::_sync_scale() {
    Vector2 *wsize = mWindow_ptr->get_window_size();
    if (wsize->y <= 640)
        mScale = 3;
    if (wsize->y == 720)
        mScale = 4;
    if (wsize->y >= 1080)
        mScale = 6;
}

void Game::change_state(GameState new_state) {
    mOldStateOrTag = mStateOrTag;
    mStateOrTag = new_state;
}

void Game::revert_state() {
    mStateOrTag = mOldStateOrTag;
}

void Game::exit_game() { mWantExit = true; }

void set_winsound_playable(bool p) { winSound.first = p; }

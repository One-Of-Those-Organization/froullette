#include "Window.hpp"
#include "../Game/GameUtils.hpp"

void Window::_apply_option(const char *title) {
    mName = title;
    mDefaultBgColor = BLACK;
    InitWindow(mSize.x, mSize.y, mName);
    InitAudioDevice();
    SetTargetFPS(mFPS);
    if (mExitKey <= -1) return;
    SetExitKey(mExitKey);
}

Window::Window(Vector2 size, const char *title) {
    mSize = size;
    mFPS = 60;
    mExitKey = -1;
    _apply_option(title);
}

Window::Window(Vector2 size, const char *title, size_t fps) {
    mSize = size;
    mFPS = fps;
    mExitKey = -1;
    _apply_option(title);
}

Window::Window(Vector2 size, const char *title, size_t fps, int exit_key) {
    mSize = size;
    mFPS = fps;
    mExitKey = exit_key;
    _apply_option(title);
}

Window::~Window() {
    UnloadImage(mAppIcon);
    CloseWindow();
    CloseAudioDevice();
}

void Window::set_window_size(Vector2 size) {
    mSize = size;
    SetWindowSize(mSize.x, mSize.y);
    mGame._sync_scale();
}

Vector2 *Window::get_window_size() { return &mSize; }

bool Window::start_window_loop() {
    mGame.init(this);
    mGame.mWindow_ptr = this;

    mState = {};
    if (mState.read_from_file(CONFIG_PATH)) {
        if (mState.mWindowSize.x > 100 || mState.mWindowSize.y > 100) {
            set_window_size(mState.mWindowSize);
            _change_res_helper(&mGame);
        }
        if (mState.mIsFullscreen) _window_flag_helper(&mGame);
    }

    while (!WindowShouldClose()) {
        if (mGame.mWantExit)
            break;
        float dt = GetFrameTime();
        _handle_key(dt);
        _handle_logic(dt);
        _handle_drawing(dt);
    }

    mState.mIsFullscreen = IsWindowFullscreen();
    if (mState.mIsFullscreen) mState.mWindowSize = mOldSize;
    else mState.mWindowSize = mSize;
    mState.write_to_file(CONFIG_PATH);

    return true;
}

void Window::_handle_drawing(float dt) {
    BeginDrawing();

    ClearBackground(mDefaultBgColor);

    mGame.handle_drawing(dt);

    EndDrawing();
}

void Window::_handle_logic(float dt) {
    mGame.mCursorPos = GetMousePosition();
    mGame.handle_logic(dt);
}

void Window::_handle_key(float dt) { mGame.handle_key(dt); }

Color Window::get_bg_color() { return mDefaultBgColor; }

void Window::set_bg_color(Color color) { mDefaultBgColor = color; }

void Window::set_fps_target(size_t fps) {
    mFPS = fps;
    SetTargetFPS(mFPS);
}
size_t Window::get_fps_target() { return mFPS; }

void Window::set_exit_key(int key) {
    mExitKey = key;
    SetExitKey(mExitKey);
}

size_t Window::get_exit_key() { return mExitKey; }

const char *Window::get_name() { return mName; }

void Window::toggle_fullscreen() {
    if (!IsWindowFullscreen()) {
        mOldSize = mSize;
        Vector2 display_size =
            Vector2{(float)GetMonitorWidth(0), (float)GetMonitorHeight(0)};
        this->set_window_size(display_size);
    } else {
        this->set_window_size(mOldSize);
    }
    ToggleFullscreen();
}

void Window::set_app_icon(const char *path) {
    mAppIcon = LoadImage(path);
    SetWindowIcon(mAppIcon);
}

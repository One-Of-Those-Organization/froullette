#include "Window/Window.hpp"

int main(void) {
    Window w(Vector2(1280, 720), "Fate Roullete", 60, KEY_F4);
    w.start_window_loop();

    return 0;
}

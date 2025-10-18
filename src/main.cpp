#include "Window/Window.hpp"

int main(void) {
    Window w(Vector2(1280, 720), 60, "Fate Roullete");

    if (!w.loop()) return 2;
    return 0;
}

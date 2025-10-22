#include "Window/Window.hpp"

int main(void) {
    const int fps = 60;
    const Vector2 size = Vector2{1280, 720};
    Window w(size, fps, "Fate Roullete");
    if (!w.loop()) return 2;
    return 0;
}

#ifndef STATE_H_
#define STATE_H_

#include <raylib.h>

struct PState {
public:
    Vector2 mWindowSize;
    bool mIsFullscreen;

    PState();
    ~PState() = default;

    bool write_to_file(const char *file_path);
    bool read_from_file(const char *file_path);
};

#endif // STATE_H_

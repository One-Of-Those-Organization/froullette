#include "State.hpp"
#include <cstdio>

PState::PState() {
    mIsFullscreen = false;
}

bool PState::write_to_file(const char *file_path) {
    FILE *f = fopen(file_path, "wb");
    if (!f) return false;

    bool status = false;
    do {
        if (fwrite("uwu\0", sizeof(char), 4, f) != 4) break;
        if (fwrite(&this->mWindowSize, sizeof(Vector2), 1, f) != 1) break;
        if (fwrite(&this->mIsFullscreen, sizeof(bool), 1, f) != 1) break;
        status = true;
    } while (false);
    fclose(f);
    return status;
}

bool PState::read_from_file(const char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) return false;
    char signature[4] = {0};

    bool status = false;
    do {
        if (fread(signature, sizeof(char), 4, f) != 4) break;
        if (fread(&this->mWindowSize, sizeof(Vector2), 1, f) != 1) break;
        if (fread(&this->mIsFullscreen, sizeof(bool), 1, f) != 1) break;
        status = true;
    } while (false);
    fclose(f);
    return status;
}

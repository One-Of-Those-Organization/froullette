#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_

#include "../ManagedSound/ManagedSound.hpp"

#include <raylib.h>
#include <vector>

struct SoundManager {
    std::vector<ManagedSound> mData;

  public:
    SoundManager();
    ~SoundManager();

    bool load_sound(const char *fp, const char *name);
    ManagedSound *get_sound(const char *name);
};

#endif // SOUND_MANAGER_H_

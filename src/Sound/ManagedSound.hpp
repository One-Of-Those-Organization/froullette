#ifndef MANAGEDSOUND_H_
#define MANAGEDSOUND_H_

#include <raylib.h>
#include <string>

struct ManagedSound {
  public:
    std::string mName;
    Sound mData;

    ManagedSound(const char *name);
    virtual ~ManagedSound();

    bool load_sound(const char *path);
    bool unload_sound();
    bool is_playing();
    void play_sound();
    void stop_sound();
};

#endif // MANAGEDSOUND_H_

#include "SoundManager.hpp"

SoundManager::SoundManager() { mData.reserve(20); }
SoundManager::~SoundManager() {
    for (size_t i = 0; i < mData.size(); i++) {
        mData[i].unload_sound();
    }
}

bool SoundManager::load_sound(const char *fp, const char *name) {
    ManagedSound s = ManagedSound(name);
    if (!s.load_sound(fp))
        return false;
    mData.push_back(s);
    return true;
}

ManagedSound *SoundManager::get_sound(const char *name) {
    for (size_t i = 0; i < mData.size(); i++) {
        auto &current = mData[i];
        if (current.mName == name) {
            return &current;
        }
    }
    return nullptr;
}

#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <cstdint>

enum class GameState : uint32_t {
    NONE     = 0,
    MENU     = 1 << 0,
    PLAYMENU = 1 << 1,
    SETTINGS = 1 << 2,
    INGAME   = 1 << 3,
    FINISHED = 1 << 4,
    TUTORIAL = 1 << 5,
    ALL      = MENU | SETTINGS | INGAME | PLAYMENU | FINISHED | TUTORIAL
};

inline GameState operator|(GameState a, GameState b) {
    return static_cast<GameState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline GameState operator&(GameState a, GameState b) {
    return static_cast<GameState>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool has_flag(GameState value, GameState flag) {
    return (value & flag) != GameState::NONE;
}

#endif // GAME_STATE_H_

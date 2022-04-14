#pragma once

#include <variant>

#include "common.hpp"

namespace sm {

struct TimedEffect {
    int remaning_frames;
};

struct StaticEffect {};

struct Effect {
    bool active;
    Position pos;
    Graphics gfx;
    std::variant<TimedEffect, StaticEffect> specific;
};

void create_cat_attack_effect(Effect& effect, Position pos);

void update_effect(Effect& effect);

void free_effect(Effect& effect);

}  // namespace sm
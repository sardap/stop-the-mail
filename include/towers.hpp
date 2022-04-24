#pragma once

#include <variant>

#include "collision.hpp"
#include "common.hpp"

namespace sm {

enum class TowerType {
    CAT = 0,
};

struct Cat {
    const static int attack_cooldown = 30;

    Collsion col;
    int current_cooldown;
};

struct Tower {
    Position pos;
    Graphics gfx;
    std::variant<Cat> specific;
    bool active;
    int damage_dealt;
};

}  // namespace sm

#pragma once

#include <variant>

#include "collision.hpp"
#include "common.hpp"

namespace sm {

struct CreateTowerArg {
    Position pos;
};

struct Cat {
    const static int attack_cooldown = 30;

    Collsion col;
    int current_cooldown;
};

struct Begal {
    const static int attack_cooldown = 60;

    Collsion col;
    int current_cooldown;
};

struct Tower {
    Position pos;
    Graphics gfx;
    std::variant<Cat, Begal> specific;
    bool active;
    int damage_dealt;
};

}  // namespace sm

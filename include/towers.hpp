#pragma once

#include "collision.hpp"
#include "common.hpp"

namespace sm {

enum class TowerType {
    CAT = 0,
};

struct Cat {
    Collsion col;
};

struct Tower {
    Position pos;
    Graphics gfx;
    std::variant<Cat> specific;
    bool active;
};

}  // namespace sm

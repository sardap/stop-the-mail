#pragma once

#include "common.hpp"

namespace sm {

enum class TowerType {
    CAT = 0,
};

struct Tower {
    Position pos;
    Graphics gfx;
    bool active;
};

}  // namespace sm

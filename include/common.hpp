#pragma once

#include <nds.h>
#include <nds/ndstypes.h>

#include <fixed/fixed.hpp>

namespace sm {

namespace globals {

extern uint32 current_frame;

}

using Fixed = math::Fixed<int32>;

static const Fixed PI = Fixed(3.14159265f);

struct Position {
    Fixed x;
    Fixed y;
};

Fixed distance(const Position& l, const Position& r);

struct Vel {
    Fixed vx;
    Fixed vy;
};

void postion_update(Position& pos, const Vel& vel);

struct Graphics {
    u16* tile;
    OamState* oam;
    int oam_id;

    ~Graphics();
};

void update_oam(Position& pos, Graphics& graphics);

struct Rectangle {
    Fixed x;
    Fixed y;
    Fixed w;
    Fixed h;
};

struct Life {
    int maxHp;
    int currentHp;
};

}  // namespace sm
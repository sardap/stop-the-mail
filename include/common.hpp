#pragma once

#include <fmt/format.h>
#include <nds.h>
#include <nds/ndstypes.h>

#include <array>
#include <fixed/fixed.hpp>

namespace sm {

namespace globals {

extern uint32 current_frame;

extern touchPosition touch_position;
extern touchPosition last_touch_position;

}  // namespace globals

using Fixed = math::Fixed<int32>;

static const Fixed PI = Fixed(3.14159265f);

struct Position {
    Fixed x;
    Fixed y;
};

inline Fixed distance(const Position& l, const Position& r) {
    auto x1 = l.x;
    auto x2 = r.x;
    auto y1 = l.y;
    auto y2 = r.y;

    return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

struct Vel {
    Fixed vx;
    Fixed vy;
};

void postion_update(Position& pos, const Vel& vel);

struct Graphics {
    u16* tile;
    OamState* oam;
    int oam_id;
    bool show;
    SpriteSize size;
    SpriteColorFormat colorFormat;

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
    Fixed maxHp;
    Fixed currentHp;
};

void apply_damage(Life& life, Fixed damage);

struct TextInfo {
    OamState* oam;
    size_t oam_offset;
    size_t oam_count;
    u16* sheet_offset;
};

struct TextGroup {
    size_t start;
    size_t count;
};

}  // namespace sm
#include <stdio.h>

#include <common.hpp>
#include <fixed/math.hpp>

namespace sm {

uint32 globals::current_frame = 0;

Fixed distance(const Position& l, const Position& r) {
    auto x1 = l.x;
    auto x2 = r.x;
    auto y1 = l.y;
    auto y2 = r.y;

    return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void postion_update(Position& pos, const Vel& vel) {
    pos.x += vel.vx;
    pos.y += vel.vy;
}

Graphics::~Graphics() {}

void update_oam(Position& pos, Graphics& graphics) {
    oamSet(
        // oam
        graphics.oam,
        // oam id
        graphics.oam_id,
        // postions
        static_cast<int>(pos.x), static_cast<int>(pos.y),
        // priority, palette
        0, 0,
        // size
        SpriteSize_16x16, SpriteColorFormat_256Color,
        graphics.tile,  // the oam gfx
        -1, false, false, false, false, false);
}

void apply_damage(Life& life, Fixed damage) { life.currentHp -= damage; }

}  // namespace sm

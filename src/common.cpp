#include <stdio.h>

#include <common.hpp>
#include <fixed/math.hpp>

namespace sm {

uint32 globals::current_frame = 0;

touchPosition globals::touch_position = touchPosition{};

touchPosition globals::last_touch_position = globals::touch_position;

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
        -1, false, !graphics.show, false, false, false);
}

void apply_damage(Life& life, Fixed damage) { life.currentHp -= damage; }

}  // namespace sm

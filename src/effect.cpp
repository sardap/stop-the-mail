#include <gfx/DefenceTowerSpritesheet.h>

#include <effect.hpp>

namespace sm {

void create_cat_attack_effect(Effect& effect, Position pos) {
    effect.active = true;
    effect.pos = pos;
    effect.gfx.size = SpriteSize_16x16;
    effect.gfx.color_format = SpriteColorFormat_256Color;
    effect.gfx.show = true;
    effect.gfx.tile = oamAllocateGfx(effect.gfx.oam, SpriteSize_16x16,
                                     SpriteColorFormat_256Color);

    u8* offset = (u8*)DefenceTowerSpritesheetTiles + (1 * (16 * 16));
    dmaCopy(offset, effect.gfx.tile, 16 * 16);

    effect.specific = TimedEffect{.remaning_frames = 20};
}

void update_effect(Effect& effect) {
    if (!effect.active) {
        return;
    }

    if (TimedEffect* timed = std::get_if<TimedEffect>(&effect.specific);
        timed) {
        if (timed->remaning_frames < 0) {
            free_effect(effect);
            return;
        }

        timed->remaning_frames--;
    }

    update_oam(effect.pos, effect.gfx);
}

void free_effect(Effect& effect) {
    effect.active = false;
    if (effect.gfx.tile != nullptr) {
        oamFreeGfx(effect.gfx.oam, effect.gfx.tile);
    }
    effect.gfx.tile = nullptr;
    oamSetHidden(effect.gfx.oam, effect.gfx.oam_id, true);
}
}  // namespace sm
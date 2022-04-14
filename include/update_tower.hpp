#pragma once

#include <gfx/towerSpritesheet.h>

#include <concepts>
#include <container.hpp>

namespace sm {

template <IsContainer T>
Tower* create_cat(T& container, Position pos, Position colPos) {
    auto* tower_ptr = container.get_free_tower();
    if (tower_ptr == nullptr) {
        return nullptr;
    }
    auto& tower = *tower_ptr;
    assert(tower.active == false);
    tower.active = true;

    auto tileOffset = 0;
    Cat& cat = std::get<Cat>(tower.specific);
    setup_collision(
        cat.col,
        Collsion::Collider{.type = Identity::Type::TOWER, .tower = &tower},
        Rectangle{.x = colPos.x, .y = colPos.y, .w = 16, .h = 16});
    container.add_collsion(&cat.col);
    cat.current_cooldown = 0;

    tower.pos = pos;
    u8* offset = (u8*)towerSpritesheetTiles + (tileOffset * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower_ptr;
}

template <IsContainer T>
void free_tower(T& container, sm::Tower& tower, int idx) {
    container.free_tower(tower, idx);
    oamSetHidden(tower.gfx.oam, tower.gfx.oam_id, true);
}

template <IsContainer T>
void update_tower(T& container, sm::Tower& tower) {
    if (!tower.active) {
        return;
    }

    update_oam(tower.pos, tower.gfx);

    if (auto* cat = std::get_if<Cat>(&tower.specific); cat) {
        if (cat->current_cooldown > 0) {
            cat->current_cooldown--;
        }

        for (size_t i = 0; i < cat->col.collisions.size(); i++) {
            if (!cat->col.collisions[i]) {
                break;
            }
            const auto& collider = cat->col.collisions[i]->object;
            switch (collider.type) {
                case Identity::Type::INVALID:
                    break;
                case Identity::Type::MAIL:
                    if (cat->current_cooldown <= 0) {
                        apply_damage(collider.mail->life, Fixed(1));
                        cat->current_cooldown = cat->attack_cooldown;
                        if (auto* effect = container.get_free_effect()) {
                            create_cat_attack_effect(
                                *effect,
                                Position{.x = collider.mail->postion.x,
                                         .y = collider.mail->postion.y});
                        }
                    }
                    break;
                case Identity::Type::TOWER:
                    break;
            }
        }

        refresh_collision(cat->col);
    }
}

}  // namespace sm
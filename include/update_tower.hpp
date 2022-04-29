#pragma once


#include <concepts>
#include <container.hpp>

namespace sm {

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
                        tower.damage_dealt++;
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
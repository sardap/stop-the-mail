#pragma once

#include <gfx/towerSpritesheet.h>

#include <container.hpp>

namespace sm {

template <IsContainer T>
Tower* create_blank(T& container, const CreateTowerArg& arg) {
    return nullptr;
}

template <IsContainer T>
Tower* create_cat(T& container, const CreateTowerArg& arg) {
    const Position& pos = arg.pos;
    const Position& col_pos = arg.colPos;

    auto* tower_ptr = container.get_free_tower();
    if (tower_ptr == nullptr) {
        return nullptr;
    }
    auto& tower = *tower_ptr;
    assert(tower.active == false);
    tower.active = true;
    tower.damage_dealt = 0;

    Cat& cat = std::get<Cat>(tower.specific);
    setup_collision(
        cat.col,
        Collsion::Collider{.type = Identity::Type::TOWER, .tower = &tower},
        Rectangle{.x = col_pos.x, .y = col_pos.y, .w = 16, .h = 16});
    container.add_collsion(&cat.col);
    cat.current_cooldown = 0;

    tower.pos = pos;
    u8* offset = (u8*)towerSpritesheetTiles + (0 * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower_ptr;
}

template <IsContainer T>
Tower* create_begal(T& container, const CreateTowerArg& arg) {
    const Position& pos = arg.pos;
    const Position& col_pos = arg.colPos;

    auto* tower_ptr = container.get_free_tower();
    if (tower_ptr == nullptr) {
        return nullptr;
    }
    auto& tower = *tower_ptr;
    assert(tower.active == false);
    tower.active = true;
    tower.damage_dealt = 0;

    Cat& cat = std::get<Cat>(tower.specific);
    setup_collision(
        cat.col,
        Collsion::Collider{.type = Identity::Type::TOWER, .tower = &tower},
        Rectangle{.x = col_pos.x, .y = col_pos.y, .w = 16, .h = 16});
    container.add_collsion(&cat.col);
    cat.current_cooldown = 0;

    tower.pos = pos;
    u8* offset = (u8*)towerSpritesheetTiles + (2 * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower_ptr;
}

}  // namespace sm